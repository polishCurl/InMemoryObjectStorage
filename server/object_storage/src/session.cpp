
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
  BOOST_LOG_TRIVIAL(debug) << "Closing session with " << getRemoteEndpointInfo()
                           << "...";
  closeSockets();
  completion_handler_();
}

void Session::start() noexcept {
  setTcpNoDelay();
  // serializer_.post([me = shared_from_this()]() { me->readFtpCommand(); });

  BOOST_LOG_TRIVIAL(debug) << "Session with " << getRemoteEndpointInfo()
                           << " started";
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
  // Close HTTP/FTP command socket
  ErrorCode error_code;
  socket_.shutdown(Socket::shutdown_both, error_code);
  socket_.close(error_code);

  // Close FTP data socket
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

void Session::getRequest() noexcept {
  /*
  boost::asio::async_read_until(
      socket_, input_stream_, "\r\n",
      serializer_.wrap([me = shared_from_this()](ErrorCode error_code,
                                                 std::size_t length) {
        if (error_code) {
          if (error_code != asio::error::eof) {
            std::cerr << "read_until error: " << error_code.message()
                      << std::endl;
          }

          me->data_acceptor_.close(ec_);

          me->data_socket_strand_.post([me]() {
            auto data_socket = me->data_socket_weakptr_.lock();
            if (data_socket) {
              asio::error_code ec_;
              data_socket->close(ec_);
            }
          });

          return;
        }

        std::istream stream(&(me->command_input_stream_));
        std::string packet_string(length - 2, ' ');
        stream.read(
            &packet_string[0],
            length - 2);  // NOLINT(readability-container-data-pointer) Reason:
                          // I need a non-const pointer here, As I am directly
                          // reading into the buffer, but .data() returns a
                          // const pointer. I don't consider a const_cast to be
                          // better. Since C++11 this is safe, as strings are
                          // stored in contiguous memeory.

        stream.ignore(2);  // Remove the "\r\n"
#ifndef NDEBUG
        std::cout << "FTP << " << packet_string << std::endl;
#endif

        me->handleFtpCommand(packet_string);
      }));
      */
}

}  // namespace object_storage
}  // namespace server
