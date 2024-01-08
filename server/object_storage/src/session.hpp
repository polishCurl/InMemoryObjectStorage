#ifndef SERVER_OBJECT_STORAGE_SRC_SESSION_HPP
#define SERVER_OBJECT_STORAGE_SRC_SESSION_HPP

#include <boost/asio.hpp>
#include <deque>
#include <functional>
#include <memory>
#include <string>

#include "filesystem/memory_fs/src/memory_fs.hpp"
#include "protocol/ftp/request/src/ftp_parser.hpp"
#include "protocol/http/request/src/http_parser.hpp"
#include "user/database/src/user_database.hpp"

using IOService = boost::asio::io_service;        ///< OS IO services
using Socket = boost::asio::ip::tcp::socket;      ///< TCP socket
using Acceptor = boost::asio::ip::tcp::acceptor;  ///< TCP connection acceptor
using HttpHandler = std::function<void(
    const protocol::http::request::HttpParser&)>;  ///< HTTP request handler

namespace server {
namespace object_storage {

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(IOService& io_service, const user::UserDatabase& user_database,
          fs::MemoryFs& filesystem,
          const std::function<void()>& completion_handler);

  // Disable copy and move since we are inheriting from shared_from_this
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;
  Session& operator=(Session&&) = delete;
  Session(Session&&) = delete;
  ~Session();

  /**
   * \brief Start session.
   */
  void start() noexcept;

  /**
   * \brief Return the socket on which this session was created.
   *
   * \return socket on which this session is running.
   */
  inline Socket& getSocket() noexcept { return socket_; }

 private:
  // ------------------ COMMON ------------------
  /**
   * \brief Disable Nagle's algorithm on HTTP/FTP command socket.
   */
  void setTcpNoDelay() noexcept;

  /**
   * \brief Close HTTP/FTP command socket.
   */
  void closeSocket() noexcept;

  /**
   * \brief Get information about remote endpoint (IP address + port number).
   *
   * \return Remote endpoint info.
   */
  std::string getRemoteEndpointInfo() const noexcept;

  /**
   * \brief Handler for receiving messages on HTTP/FTP command socket.
   *
   * \note This method is asynchronous.
   */
  void receiveMessageHandler() noexcept;

  /**
   * \brief Handler for sending messages on HTTP/FTP command socket.
   *
   * \note This method is asynchronous.
   */
  void sendMessageHandler() noexcept;

  /**
   * \brief Send message on HTTP/FTP command socket.
   *
   * \param message Message to send.
   */
  void sendMessage(const std::string& message);

  // ------------------ FTP ------------------
  /**
   * \brief Close FTP data socket.
   */
  void closeFtpDataSocket() noexcept;

  /**
   * \brief Handle FTP request.
   *
   * \param request Request to handle.
   */
  void handleFtpRequest(const std::string& request) noexcept;

  // ------------------ HTTP ------------------
  /**
   * \brief Handle HTTP request.
   *
   * \param request Request to handle.
   */
  void handleHttpRequest(std::string& request) noexcept;

  /**
   * \brief Handle HTTP GET request.
   */
  void handleHttpGet(const protocol::http::request::HttpParser& parser);

  /**
   * \brief Handle HTTP PUT request.
   */
  void handleHttpPut(const protocol::http::request::HttpParser& parser);

  /**
   * \brief Handle HTTP DELETE request.
   */
  void handleHttpDelete(const protocol::http::request::HttpParser& parser);

  // ------------------ COMMON ------------------
  const std::function<void()> completion_handler_;  ///< Completion handler
  const user::UserDatabase& user_database_;         ///< User database
  fs::MemoryFs& filesystem_;                        ///< In-memory file storage
  IOService& io_service_;                           ///< OS IO services
  Socket socket_;                                   ///< HTTP/FTP command socket

  /// Serializer for handler execution on HTTP/FTP command socket
  IOService::strand serializer_;

  /// Buffer for reading messages from HTTP/FTP socket
  boost::asio::streambuf input_stream_;

  /// Output message queue storing HTTP/FTP responses ready to be sent.
  std::deque<std::string> output_queue_;

  // ------------------ FTP ------------------
  /// Acceptor for FTP data socket connections
  Acceptor ftp_data_acceptor_;

  /// Serializer for handler execution on FTP data socket
  IOService::strand ftp_data_serializer_;

  /// FTP data socket
  std::weak_ptr<Socket> ftp_data_socket_;

  /// Output data queue
  std::deque<std::shared_ptr<fs::File>> ftp_data_buffer_;

  /// Currently logged in user.
  std::shared_ptr<user::User> ftp_user_;

  // ------------------ HTTP ------------------
  /// Mapping from HTTP request method to the corresponding handler function.
  const std::unordered_map<protocol::http::request::HttpMethod, HttpHandler>
      http_handlers_;
};
}  // namespace object_storage
}  // namespace server

#endif  // SERVER_OBJECT_STORAGE_SRC_SESSION_HPP
