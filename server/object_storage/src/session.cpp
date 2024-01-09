
#include "session.hpp"

#include <boost/log/trivial.hpp>

#include "protocol/detector/src/protocol_detector.hpp"
#include "protocol/ftp/response/src/ftp_response.hpp"
#include "protocol/http/response/src/http_response.hpp"

using ErrorCode = boost::system::error_code;
using protocol::detector::AppLayerProtocol;
using protocol::detector::detectProtocol;
using protocol::ftp::request::FtpParser;
using protocol::ftp::response::FtpReplyCode;
using protocol::ftp::response::FtpResponse;
using protocol::http::request::HttpMethod;
using protocol::http::request::HttpParser;
using protocol::http::response::HttpResource;
using protocol::http::response::HttpResponse;
using protocol::http::response::HttpResponseHeaders;
using protocol::http::response::HttpStatus;
using user::User;

namespace server {
namespace object_storage {

Session::Session(IOService& io_service, const user::UserDatabase& user_database,
                 bool authenticate, fs::MemoryFs& filesystem,
                 const std::function<void()>& completion_handler)
    :  // ------------------ COMMON ------------------
      completion_handler_{completion_handler},
      user_database_{user_database},
      authenticate_{authenticate},
      filesystem_{filesystem},
      io_service_{io_service},
      socket_{io_service_},
      serializer_{io_service_},
      // ------------------ FTP ------------------
      ftp_data_acceptor_{io_service},
      ftp_data_serializer_{io_service},
      http_handlers_{
          {HttpMethod::Get,
           std::bind(&Session::handleHttpGet, this, std::placeholders::_1)},
          {HttpMethod::Put,
           std::bind(&Session::handleHttpPut, this, std::placeholders::_1)},
          {HttpMethod::Delete,
           std::bind(&Session::handleHttpDelete, this, std::placeholders::_1)},
      } {}

Session::~Session() {
  closeSocket();
  closeFtpDataSocket();
  completion_handler_();
}

void Session::start() noexcept {
  BOOST_LOG_TRIVIAL(debug) << "Starting session with "
                           << getRemoteEndpointInfo();
  setTcpNoDelay();
  serializer_.post(
      [me = shared_from_this()]() { me->receiveMessageHandler(); });
}

void Session::setTcpNoDelay() noexcept {
  ErrorCode error_code;
  socket_.set_option(boost::asio::ip::tcp::no_delay(true), error_code);
  if (error_code)
    BOOST_LOG_TRIVIAL(error)
        << "Failed to set HTTP/FTP socket option TCP_NODELAY:"
        << error_code.message();
}

void Session::closeSocket() noexcept {
  ErrorCode error_code;
  socket_.shutdown(Socket::shutdown_both, error_code);
  socket_.close(error_code);
}

std::string Session::getRemoteEndpointInfo() const noexcept {
  return socket_.remote_endpoint().address().to_string() + ":" +
         std::to_string(socket_.remote_endpoint().port());
}

void Session::receiveMessageHandler() noexcept {
  // Asynchronously read input request until CRLF symbol is found. The CRLF is
  // shared between HTTP (end of request line) and FTP (end of entire
  // request).
  boost::asio::async_read_until(
      socket_, input_stream_, "\r\n",
      serializer_.wrap([me = shared_from_this()](ErrorCode error_code,
                                                 std::size_t header_length) {
        if (error_code) {
          if (error_code == boost::asio::error::eof) {
            BOOST_LOG_TRIVIAL(info)
                << "Connection closed by " << me->getRemoteEndpointInfo();
          } else {
            BOOST_LOG_TRIVIAL(error)
                << "Failed to read HTTP/FTP request header: "
                << error_code.message();
          }

          me->ftp_data_acceptor_.close(error_code);
          me->ftp_data_serializer_.post([me]() { me->closeFtpDataSocket(); });

        } else {
          // Process packet to determine if if is HTTP or FTP.
          std::istream stream(&(me->input_stream_));
          std::string packet;
          packet.resize(header_length);
          stream.read(&packet[0], header_length);
          const auto app_layer_protocol = detectProtocol(packet);

          switch (app_layer_protocol) {
            case AppLayerProtocol::Http:
              me->handleHttpRequest(packet);
              break;

            case AppLayerProtocol::Ftp:
              me->handleFtpRequest(packet);
              break;
          }

          me->receiveMessageHandler();
        }
      }));
}

void Session::sendMessageHandler() noexcept {
  BOOST_LOG_TRIVIAL(debug) << "Sending message:\n" << output_queue_.front();

  // Get the next message from send queue and send it asynchronously.
  boost::asio::async_write(
      socket_, boost::asio::buffer(output_queue_.front()),
      serializer_.wrap(
          [me = shared_from_this()](ErrorCode error_code, std::size_t) {
            if (!error_code) {
              me->output_queue_.pop_front();

              // If there are more messages to send, trigger this handler again.
              // Otherwise, the thread enqueueing a new message will trigger
              // this handler.
              const auto write_in_progress = !me->output_queue_.empty();
              if (write_in_progress) {
                me->sendMessageHandler();
              }
            } else {
              BOOST_LOG_TRIVIAL(error)
                  << "Failed to send message:\n"
                  << me->output_queue_.front() << error_code.message();
            }
          }));
}

void Session::sendMessage(const std::string& message) {
  // Put the message to the send queue. If the send queue was empty before
  // manually trigger the asynchronous send handler execution.
  serializer_.post([me = shared_from_this(), message]() {
    const bool write_in_progress = !me->output_queue_.empty();
    me->output_queue_.push_back(message);
    if (!write_in_progress) {
      me->sendMessageHandler();
    }
  });
}

void Session::closeFtpDataSocket() noexcept {
  ErrorCode error_code;
  auto data_socket = ftp_data_socket_.lock();
  if (data_socket) {
    data_socket->shutdown(Socket::shutdown_both, error_code);
    data_socket->close(error_code);
  }
}

void Session::handleFtpRequest(const std::string& request) noexcept {
  BOOST_LOG_TRIVIAL(debug) << "FTP request:\n" << request;
}

void Session::handleHttpRequest(std::string& request) noexcept {
  auto status_line_size = request.size();

  // Read the rest of the HTTP request (excluding message body)
  auto response_headers_size =
      boost::asio::read_until(socket_, input_stream_, "\r\n\r\n");
  std::istream stream(&input_stream_);
  request.resize(request.size() + response_headers_size);
  stream.read(&request[status_line_size], response_headers_size);

  BOOST_LOG_TRIVIAL(debug) << "HTTP request:\n" << request;

  // If request is valid, delegate it to the right handler based on the HTTP
  // method.
  HttpParser parser{request};
  if (!parser.isValid()) {
    BOOST_LOG_TRIVIAL(error) << "Failed to parse HTTP request:\n" << request;
    sendMessage(HttpResponse{HttpStatus::BadRequest});

  } else if (!authenticateHttpUser(parser)) {
    sendMessage(HttpResponse{HttpStatus::Unauthorized,
                             HttpResponseHeaders{{"WWW-Authenticate", "Basic"},
                                                 {"Content-Length", "0"}}});
  } else {
    http_handlers_.at(parser.getMethod())(parser);
  }
}

bool Session::authenticateHttpUser(const HttpParser& parser) noexcept {
  if (authenticate_) {
    const auto auth_info = parser.getAuthInfo();
    user::User user_to_auth{auth_info->username, auth_info->password};
    const auto user_exists = user_database_.exists(user_to_auth);

    if (!user_exists) {
      BOOST_LOG_TRIVIAL(error)
          << "Failed to authenticate user " << user_to_auth;
    }

    return user_exists;
  }

  return true;
}

void Session::handleHttpGet(const HttpParser& parser) {
  if (parser.getUri() == "/") {
    // If request has 'GET /' format, list all files stored in the filesystem.
    const auto file_list = filesystem_.list();
    std::string response;
    for (const auto& file : file_list) {
      response += file + '\n';
    }
    sendMessage(HttpResponse{HttpStatus::Ok, response});

  } else {
    // Otherwise, get the file from the filesystem and send it in response (if
    // it was found).
    const auto [status, file] = filesystem_.get(std::string{parser.getUri()});
    switch (status) {
      case fs::Status::Success:
        sendMessage(HttpResponse{HttpStatus::Ok, file});
        break;
      case fs::Status::FileNotFound:
        sendMessage(HttpResponse{HttpStatus::NotFound});
        break;
      default:
        sendMessage(HttpResponse{HttpStatus::InternalServerError});
        break;
    }
  }
}

void Session::handleHttpPut(const HttpParser& parser) {
  ErrorCode error_code;
  const auto file_size = parser.getResourceSize();
  const auto filename = std::string{parser.getUri()};

  // Read the HTTP message body containing the content/resource/file.
  auto bytes_read =
      boost::asio::read(socket_, input_stream_,
                        boost::asio::transfer_exactly(file_size), error_code);

  if (bytes_read != file_size) {
    BOOST_LOG_TRIVIAL(error) << "Failed to read " << file_size
                             << " bytes (Actual: " << bytes_read << ')';
    sendMessage(HttpResponse{HttpStatus::BadRequest});
  } else {
    auto bufs = input_stream_.data();
    fs::File file(boost::asio::buffers_begin(bufs),
                  boost::asio::buffers_begin(bufs) + file_size);

    const auto status = filesystem_.add(std::string{filename}, file);
    switch (status) {
      case fs::Status::Success:
        BOOST_LOG_TRIVIAL(info) << "Saved file: " << filename;
        sendMessage(HttpResponse{HttpStatus::Created});
        break;
      case fs::Status::AlreadyExists:
        sendMessage(HttpResponse{HttpStatus::NotFound});
        break;
      default:
        sendMessage(HttpResponse{HttpStatus::InternalServerError});
        break;
    }
  }
}

void Session::handleHttpDelete(const HttpParser& parser) {
  const auto& filename = std::string{parser.getUri()};
  const auto status = filesystem_.remove(filename);
  switch (status) {
    case fs::Status::Success:
      BOOST_LOG_TRIVIAL(info) << "Deleted file: " << filename;
      sendMessage(HttpResponse{HttpStatus::Ok});
      break;
    case fs::Status::FileNotFound:
      sendMessage(HttpResponse{HttpStatus::NotFound});
      break;
    default:
      sendMessage(HttpResponse{HttpStatus::InternalServerError});
      break;
  }
}

}  // namespace object_storage
}  // namespace server
