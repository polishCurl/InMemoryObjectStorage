#ifndef SERVER_OBJECT_STORAGE_SRC_SESSION_HPP
#define SERVER_OBJECT_STORAGE_SRC_SESSION_HPP

#include <boost/asio.hpp>
#include <deque>
#include <memory>

#include "filesystem/memory_fs/src/memory_fs.hpp"
#include "user/database/src/user_database.hpp"

namespace server {

namespace object_storage {

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(boost::asio::io_service& io_service,
          const user::UserDatabase& user_database,
          const fs::MemoryFs& filesystem,
          const std::function<void()>& completion_handler);

  // Disable copy and move since we are inheriting from shared_from_this
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;
  Session& operator=(Session&&) = delete;
  Session(Session&&) = delete;
  ~Session();

  /**
   * \brief Return the socket on which this session was created.
   *
   * \return Socket on which this session is running.
   */
  inline boost::asio::ip::tcp::socket& getSocket() noexcept { return socket_; }

 protected:
  // ------------------ COMMON ------------------
  const std::function<void()> completion_handler_;  ///< Completion handler
  const user::UserDatabase& user_database_;         ///< User database
  const fs::MemoryFs& filesystem_;                  ///< In-memory file storage
  boost::asio::io_service& io_service_;             ///< OS IO services
  boost::asio::ip::tcp::socket socket_;             ///< HTTP/FTP command socket

  /// Serializer for handler execution on HTTP/FTP command socket
  boost::asio::io_service::strand serializer_;

  /// Buffer for reading messages from HTTP/FTP socket
  boost::asio::streambuf input_stream_;

  /// Output message queue
  std::deque<std::string> output_queue_;

  // ------------------ FTP ------------------
  /// Acceptor for FTP data socket connections
  boost::asio::ip::tcp::acceptor ftp_data_acceptor_;

  /// Serializer for handler execution on FTP data socket
  boost::asio::io_service::strand ftp_data_serializer_;

  /// FTP data socket
  std::weak_ptr<boost::asio::ip::tcp::socket> ftp_data_socket_;

  /// Output data queue
  std::deque<std::shared_ptr<fs::File>> ftp_data_buffer_;

  std::shared_ptr<user::User> ftp_user_;  ///< Currently logged in user.

  // ------------------ HTTP ------------------

 private:
  void setTcpNoDelay() noexcept;
  void closeSockets() noexcept;
};
}  // namespace object_storage
}  // namespace server

#endif  // SERVER_OBJECT_STORAGE_SRC_SESSION_HPP
