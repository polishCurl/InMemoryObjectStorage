
#include "session.hpp"

#include <boost/log/trivial.hpp>

#include "protocol/detector/src/protocol_detector.hpp"

using ErrorCode = boost::system::error_code;
using protocol::detector::AppLayerProtocol;
using protocol::detector::detectProtocol;
using protocol::ftp::request::FtpParser;
using protocol::http::request::HttpMethod;
using protocol::http::request::HttpParser;

namespace server {
namespace object_storage {

Session::Session(IOService& io_service, const user::UserDatabase& user_database,
                 const fs::MemoryFs& filesystem,
                 const std::function<void()>& completion_handler)
    :  // ------------------ COMMON ------------------
      completion_handler_{completion_handler},
      user_database_{user_database},
      filesystem_{filesystem},
      io_service_{io_service},
      socket_{io_service_},
      serializer_{io_service_},
      // ------------------ FTP ------------------
      ftp_data_acceptor_{io_service},
      ftp_data_serializer_(io_service),
      http_handlers_{
          {HttpMethod::Get,
           std::bind(&Session::handleHttpGet, this, std::placeholders::_1)},
          {HttpMethod::Put,
           std::bind(&Session::handleHttpPut, this, std::placeholders::_1)},
          {HttpMethod::Delete,
           std::bind(&Session::handleHttpDelete, this, std::placeholders::_1)},
      }

{}

Session::~Session() {
  closeSocket();
  closeFtpDataSocket();
  completion_handler_();
}

void Session::start() noexcept {
  BOOST_LOG_TRIVIAL(debug) << "Starting session with "
                           << getRemoteEndpointInfo();
  setTcpNoDelay();
  serializer_.post([me = shared_from_this()]() { me->readRequest(); });
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

void Session::closeFtpDataSocket() noexcept {
  ErrorCode error_code;
  auto data_socket = ftp_data_socket_.lock();
  if (data_socket) {
    data_socket->shutdown(Socket::shutdown_both, error_code);
    data_socket->close(error_code);
  }
}

std::string Session::getRemoteEndpointInfo() const noexcept {
  return socket_.remote_endpoint().address().to_string() + ":" +
         std::to_string(socket_.remote_endpoint().port());
}

void Session::readRequest() noexcept {
  // Asynchronously read input request until CRLF symbol is found. The CRLF is
  // shared between HTTP (end of request line) and FTP (end of entire
  // request).
  boost::asio::async_read_until(
      socket_, input_stream_, "\r\n",
      serializer_.wrap([me = shared_from_this()](ErrorCode error_code,
                                                 std::size_t header_length) {
        if (error_code) {
          if (error_code == boost::asio::error::eof) {
            BOOST_LOG_TRIVIAL(error)
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

          me->readRequest();
        }
      }));
}

void Session::handleHttpRequest(std::string& request) noexcept {
  auto header_length = request.size();

  // Read the rest of the HTTP request (excluding message body)
  auto body_length =
      boost::asio::read_until(socket_, input_stream_, "\r\n\r\n");
  std::istream stream(&input_stream_);
  request.resize(request.size() + body_length);
  stream.read(&request[header_length], body_length);

  BOOST_LOG_TRIVIAL(debug) << "HTTP request:\n" << request;

  HttpParser parser{request};
  if (!parser.isValid()) {
    BOOST_LOG_TRIVIAL(error) << "Failed to parse HTTP request:\n" << request;
    return;
  }

  http_handlers_.at(parser.getMethod())(parser);
}

void Session::handleFtpRequest(const std::string& request) noexcept {
  BOOST_LOG_TRIVIAL(debug) << "FTP request:\n" << request;
}

void Session::handleHttpGet(const HttpParser& parser) {
  BOOST_LOG_TRIVIAL(debug) << "HTTP GET";
  // const auto file = filesystem_.get(parser.getUri());
}

void Session::handleHttpPut(const HttpParser& parser) {
  BOOST_LOG_TRIVIAL(debug) << "HTTP PUT";
}

void Session::handleHttpDelete(const HttpParser& parser) {
  BOOST_LOG_TRIVIAL(debug) << "HTTP DELETE";
  // const auto result = filesystem_.remove(parser.getUri());
}

}  // namespace object_storage
}  // namespace server
