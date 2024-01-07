
#include "session.hpp"

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <memory>

using ErrorCode = boost::system::error_code;

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
      ftp_data_serializer_(io_service) {}

Session::~Session() {
  closeSockets();
  completion_handler_();
}

void Session::start() noexcept {
  BOOST_LOG_TRIVIAL(debug) << "Starting session with "
                           << getRemoteEndpointInfo();
  setTcpNoDelay();
  serializer_.post([me = shared_from_this()]() { me->getNextRequest(); });
}

void Session::setTcpNoDelay() noexcept {
  ErrorCode error_code;
  socket_.set_option(boost::asio::ip::tcp::no_delay(true), error_code);
  if (error_code)
    BOOST_LOG_TRIVIAL(error)
        << "Failed to set HTTP/FTP socket option TCP_NODELAY:"
        << error_code.message();
}

void Session::closeSockets() noexcept {
  closeHttpFtpSocket();
  closeFtpDataSocket();
}

void Session::closeHttpFtpSocket() noexcept {
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

void Session::getNextRequest() noexcept {
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
          std::istream stream(&(me->input_stream_));
          std::string packet;
          packet.resize(header_length);
          stream.read(&packet[0], header_length);

          auto body_length = boost::asio::read_until(
              me->socket_, me->input_stream_, "\r\n\r\n");

          packet.resize(header_length + body_length);
          stream.read(&packet[header_length], body_length);

          BOOST_LOG_TRIVIAL(debug) << "FTP/HTTP header:\n" << packet;
          me->getNextRequest();
        }
      }));
}

}  // namespace object_storage
}  // namespace server
