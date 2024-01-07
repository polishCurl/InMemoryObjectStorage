
#include "session.hpp"

#include <boost/asio.hpp>
#include <boost/log/trivial.hpp>
#include <memory>

using ErrorCode = boost::system::error_code;

namespace server {

namespace object_storage {

Session::Session(boost::asio::io_service& io_service,
                 const user::UserDatabase& user_database,
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
      ftp_data_serializer_(io_service) {
  setTcpNoDelay();
  // serializer_.post([me = shared_from_this()]() { me->readFtpCommand(); });

  BOOST_LOG_TRIVIAL(debug) << "Session started";
}

Session::~Session() {
  BOOST_LOG_TRIVIAL(debug) << "Closing session...";
  closeSockets();
  completion_handler_();
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
  socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, error_code);
  socket_.close(error_code);

  // Close FTP data socket
  auto data_socket = ftp_data_socket_.lock();
  if (data_socket) {
    data_socket->shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                          error_code);
    data_socket->close(error_code);
  }
}

}  // namespace object_storage
}  // namespace server
