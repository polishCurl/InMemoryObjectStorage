
#include "session.hpp"

#include <boost/log/trivial.hpp>

#include "protocol/detector/src/protocol_detector.hpp"
#include "protocol/ftp/response/src/ftp_response.hpp"
#include "protocol/http/response/src/http_response.hpp"

using ErrorCode = boost::system::error_code;
using protocol::detector::AppLayerProtocol;
using protocol::detector::detectProtocol;
using protocol::ftp::request::FtpCommand;
using protocol::ftp::request::FtpParser;
using protocol::ftp::response::FtpReplyCode;
using protocol::ftp::response::FtpResponse;
using protocol::http::request::HttpMethod;

using user::User;

namespace server {
namespace object_storage {

Session::Session(IOService& io_service, const user::UserDatabase& user_database,
                 bool authenticate, fs::MemoryFs& filesystem,
                 PortRange ftp_port_range,
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
      ftp_port_range_{ftp_port_range},
      last_ftp_command_{FtpCommand::Unrecognized},
      current_working_dir_{'/'},
      ftp_handlers_{
          {FtpCommand::User,
           std::bind(&Session::handleFtpUser, this, std::placeholders::_1)},
          {FtpCommand::Pass,
           std::bind(&Session::handleFtpPass, this, std::placeholders::_1)},
          {FtpCommand::List,
           std::bind(&Session::handleFtpList, this, std::placeholders::_1)},
          {FtpCommand::Retr,
           std::bind(&Session::handleFtpRetr, this, std::placeholders::_1)},
          {FtpCommand::Stor,
           std::bind(&Session::handleFtpStor, this, std::placeholders::_1)},
          {FtpCommand::Dele,
           std::bind(&Session::handleFtpDele, this, std::placeholders::_1)},
          {FtpCommand::Pasv,
           std::bind(&Session::handleFtpPasv, this, std::placeholders::_1)},
          {FtpCommand::Type,
           std::bind(&Session::handleFtpType, this, std::placeholders::_1)},
          {FtpCommand::Quit,
           std::bind(&Session::handleFtpQuit, this, std::placeholders::_1)},
          {FtpCommand::Cwd,
           std::bind(&Session::handleFtpCwd, this, std::placeholders::_1)},
      },
      // ------------------ HTTP ------------------
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

  // Send 'ready for new user' message only if we know that the client is FTP.
  const auto client_port = socket_.remote_endpoint().port();
  if ((client_port >= ftp_port_range_.min_port) &&
      (client_port <= ftp_port_range_.max_port))
    sendMessage(FtpResponse(FtpReplyCode::SERVICE_READY_FOR_NEW_USER,
                            "Welcome to ObjectStorage server"));
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
          BOOST_LOG_TRIVIAL(debug) << "Received message:\n" << packet;
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

void Session::sendFtpDataHandler(
    const std::shared_ptr<Socket>& data_socket) noexcept {
  ftp_data_serializer_.post([me = shared_from_this(), data_socket]() {
    // Get the next file from FTP data socket send queue.
    const auto data = me->ftp_data_buffer_.front();

    if (data) {
      // If the file is non-empty, send if over FTP data socket and retrigger
      // this handler so that the next file can be sent.
      boost::asio::async_write(
          *data_socket, boost::asio::buffer(*data),
          me->ftp_data_serializer_.wrap(
              [me, data, data_socket](ErrorCode error_code, std::size_t) {
                me->ftp_data_buffer_.pop_front();

                if (error_code) {
                  BOOST_LOG_TRIVIAL(error)
                      << "Failed to write data: " << error_code.message();
                  return;
                }

                if (!me->ftp_data_buffer_.empty()) {
                  me->sendFtpDataHandler(data_socket);
                }
              }));
    } else {
      // If the file to send is empty, this means transmission end.
      // Close the FTP data socket.
      me->ftp_data_buffer_.pop_front();
      ErrorCode error_code;
      data_socket->shutdown(Socket::shutdown_both, error_code);
      data_socket->close(error_code);

      me->sendMessage(
          FtpResponse(FtpReplyCode::CLOSING_DATA_CONNECTION, "Done"));
    }
  });
}

void Session::enqueueFtpDataHandler(
    const std::shared_ptr<fs::File>& file,
    const std::shared_ptr<Socket>& data_socket) noexcept {
  ftp_data_serializer_.post([me = shared_from_this(), file, data_socket]() {
    // Enqueue the file for sending and trigger the send handler if the are no
    // pending writes.
    const auto write_in_progress = (!me->ftp_data_buffer_.empty());
    me->ftp_data_buffer_.push_back(file);
    if (!write_in_progress) {
      me->sendFtpDataHandler(data_socket);
    }
  });
}

void Session::acceptFtpData(
    const std::shared_ptr<fs::File>& file,
    const std::shared_ptr<std::string>& filepath) noexcept {
  auto data_socket = std::make_shared<Socket>(io_service_);

  // Once the connection request comes, start asynchronously receiving the file.
  ftp_data_acceptor_.async_accept(
      *data_socket,
      ftp_data_serializer_.wrap([data_socket, file, filepath,
                                 me = shared_from_this()](auto error_code) {
        if (error_code) {
          me->sendMessage(
              FtpResponse(FtpReplyCode::TRANSFER_ABORTED,
                          "Data transfer aborted: " + error_code.message()));
          return;
        }

        me->ftp_data_socket_ = data_socket;
        me->receiveData(file, filepath, data_socket);
      }));
}

void Session::receiveData(const std::shared_ptr<fs::File>& file,
                          const std::shared_ptr<std::string>& filepath,
                          const std::shared_ptr<Socket>& socket) noexcept {
  auto buffer = std::make_shared<fs::File>();
  buffer->resize(1024 * 1024 * 1);

  boost::asio::async_read(
      *socket, boost::asio::buffer(*buffer),
      boost::asio::transfer_at_least(buffer->size()),
      ftp_data_serializer_.wrap(
          [me = shared_from_this(), file, filepath, socket, buffer](
              ErrorCode error_code, std::size_t length) {
            buffer->resize(length);
            if (error_code) {
              if (length > 0) {
                file->append(*buffer);
              }
              me->saveFtpData(file, filepath);
            } else if (length > 0) {
              me->receiveData(file, filepath, socket);
              file->append(*buffer);
            }
          }));
}

void Session::saveFtpData(
    const std::shared_ptr<fs::File>& file,
    const std::shared_ptr<std::string>& filepath) noexcept {
  ftp_data_serializer_.post([me = shared_from_this(), file, filepath]() {
    const auto status = me->filesystem_.add(*filepath, *file);
    switch (status) {
      case fs::Status::Success:
        BOOST_LOG_TRIVIAL(info) << "Saved file: " << *filepath;
        me->sendMessage(
            FtpResponse{FtpReplyCode::CLOSING_DATA_CONNECTION, "File saved"});
        break;
      default:
        me->sendMessage(
            FtpResponse(FtpReplyCode::FILE_ACTION_NOT_TAKEN, "File not saved"));
        break;
    }

    me->closeFtpDataSocket();
  });
}

bool Session::setUpFtpDataConnectionAcceptor() noexcept {
  ErrorCode error_code;
  if (ftp_data_acceptor_.is_open()) {
    ftp_data_acceptor_.close(error_code);
    if (error_code) {
      BOOST_LOG_TRIVIAL(error)
          << "Error closing data acceptor: " << error_code.message();
    }
  }

  const Endpoint endpoint{boost::asio::ip::tcp::v4(), 0};

  ftp_data_acceptor_.open(endpoint.protocol(), error_code);
  if (error_code) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to open FTP data acceptor: " << error_code.message();

    return false;
  }

  ftp_data_acceptor_.bind(endpoint, error_code);
  if (error_code) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to bind FTP data acceptor: " << error_code.message();
    return false;
  }

  ftp_data_acceptor_.listen(boost::asio::socket_base::max_connections,
                            error_code);
  if (error_code) {
    BOOST_LOG_TRIVIAL(error) << "Failed to listen for FTP data connections: "
                             << error_code.message();
    return false;
  }

  return true;
}

}  // namespace object_storage
}  // namespace server
