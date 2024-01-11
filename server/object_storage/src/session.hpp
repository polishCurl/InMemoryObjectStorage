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

namespace server {
namespace object_storage {

using IOService = boost::asio::io_service;        ///< OS IO services
using Socket = boost::asio::ip::tcp::socket;      ///< TCP socket
using Acceptor = boost::asio::ip::tcp::acceptor;  ///< TCP connection acceptor
using HttpHandler = std::function<void(
    const protocol::http::request::HttpParser&)>;  ///< HTTP request handler
using FtpHandler = std::function<void(
    const protocol::ftp::request::FtpParser&)>;   ///< FTP request handler
using ErrorCode = boost::system::error_code;      ///< Error code
using Endpoint = boost::asio::ip::tcp::endpoint;  ///< TCP endpoint

/**
 * \brief Range of port ID values.
 */
struct PortRange {
  std::uint16_t min_port;  ///< Minimum port ID in the range.
  std::uint16_t max_port;  ///< Maximum port ID in the range.
};

class Session : public std::enable_shared_from_this<Session> {
 public:
  /**
   * \brief Create a new session.
   *
   * \param io_service OS IO services.
   * \param user_database Users recognized by the server.
   * \param authenticate Enable/disable user authentication.
   * \param filesystem Filesystem to manage.
   * \param ftp_port_range Port numbers to use by clients for FTP.
   */
  Session(IOService& io_service, const user::UserDatabase& user_database,
          bool authenticate, fs::MemoryFs& filesystem,
          PortRange ftp_port_range);

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
   * \brief Disable Nagle's algorithm on HTTP/FTP socket.
   */
  void setTcpNoDelay() noexcept;

  /**
   * \brief Close HTTP/FTP socket.
   */
  void closeSocket() noexcept;

  /**
   * \brief Get information about remote endpoint (IP address + port number).
   *
   * \return Remote endpoint info.
   */
  std::string getRemoteEndpointInfo() const noexcept;

  /**
   * \brief Handler for receiving messages on HTTP/FTP socket.
   *
   * \note This method is asynchronous.
   */
  void receiveMessage() noexcept;

  /**
   * \brief Take the next message from send queue and transmit it.
   *
   * \note This method is asynchronous.
   */
  void sendMessageHandler() noexcept;

  /**
   * \brief Send message on HTTP/FTP socket.
   *
   * \note The message is put on the send queue and will be transmitted
   * asynchronously.
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
   * \brief Handler for sending FTP data.
   *
   * \note This method is asynchronous.
   *
   * \param data_socket Socket on which to send.
   */
  void sendFtpDataHandler(const std::shared_ptr<Socket>& data_socket) noexcept;

  /**
   * \brief Handler for enqueueing data to FTP-send buffer.
   *
   * \note This method is asynchronous.
   *
   * \param file File to send.
   * \param data_socket Socket on which the data will be sent.
   */
  void enqueueFtpDataHandler(
      const std::shared_ptr<fs::File>& file,
      const std::shared_ptr<Socket>& data_socket) noexcept;

  /**
   * \brief Listen on FTP data socket for requests to upload a file.
   *
   * \note This method is asynchronous.
   *
   * \param file Buffer where the incoming file will be saved.
   * \param filepath Path where the received file will be saved.
   */
  void acceptFile(const std::shared_ptr<fs::File>& file,
                  const std::shared_ptr<std::string>& filepath) noexcept;

  /**
   * \brief Receive data on the given socket.
   *
   * \note This method is asynchronous.
   *
   * \param file Buffer where the received file will be saved.
   * \param filepath Path where the received file will be saved.
   * \param socket Socket on which the data will be received.
   */
  void receiveFile(const std::shared_ptr<fs::File>& file,
                   const std::shared_ptr<std::string>& filepath,
                   const std::shared_ptr<Socket>& socket) noexcept;

  /**
   * \brief Save file to the filesystem.
   *
   * \note This method is asynchronous.
   *
   * \param file File to save.
   * \param filepath Path in the filesystem, where the file will be saved.
   */
  void saveFile(const std::shared_ptr<fs::File>& file,
                const std::shared_ptr<std::string>& filepath) noexcept;

  /**
   * \brief Handle FTP request.
   *
   * \param request Request to handle.
   */
  void handleFtp(const std::string& request) noexcept;

  /**
   * \brief Handle FTP USER command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpUser(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP PASS command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpPass(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP LIST command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpList(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP RETR command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpRetr(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP STOR command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpStor(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP DELE command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpDele(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP PASV command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpPasv(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP TYPE command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpType(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP QUIT command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpQuit(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Handle FTP CWD command.
   *
   * \param parser Parsed FTP request.
   */
  void handleFtpCwd(const protocol::ftp::request::FtpParser& parser);

  /**
   * \brief Set up connection acceptor on FTP data socket.
   *
   * \return True if server is ready to accept connections, false otherwise.
   */
  bool configureDataAcceptor() noexcept;

  // ------------------ HTTP ------------------
  /**
   * \brief Handle HTTP request.
   *
   * \param request Request to handle.
   */
  void handleHttp(std::string& request) noexcept;

  /**
   * \brief Authenticate HTTP user.
   *
   * Use HTTP basic authentication to check if user with given username and
   * password can access the server.
   *
   * \note There is only a single HTTP realm defined. User has either R/W
   * access to all server resources or no access at all.
   *
   * \param parser Parsed HTTP request.
   *
   * \return True if user was authenticated successfully, false otherwise.
   */
  bool authHttpUser(const protocol::http::request::HttpParser& parser) noexcept;

  /**
   * \brief Handle HTTP GET request.
   *
   * \param parser Parsed HTTP request.
   */
  void handleHttpGet(const protocol::http::request::HttpParser& parser);

  /**
   * \brief Handle HTTP PUT request.
   *
   * \param parser Parsed HTTP request.
   */
  void handleHttpPut(const protocol::http::request::HttpParser& parser);

  /**
   * \brief Handle HTTP DELETE request.
   *
   * \param parser Parsed HTTP request.
   */
  void handleHttpDelete(const protocol::http::request::HttpParser& parser);

  // ------------------ COMMON ------------------
  const user::UserDatabase& user_database_;  ///< User database
  const bool authenticate_;                  ///< Authenticate users
  fs::MemoryFs& filesystem_;                 ///< In-memory file storage
  IOService& io_service_;                    ///< OS IO services
  Socket socket_;                            ///< HTTP/FTP socket

  /// Serializer for handler execution on HTTP/FTP socket
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

  /// Port numbers to use for FTP.
  PortRange ftp_port_range_;

  /// Last username provided in the FTP session.
  std::string last_username_;

  /// Currently logged in user.
  std::optional<user::User> logged_in_user_;

  /// Last FTP command executed.
  protocol::ftp::request::FtpCommand last_ftp_command_;

  /// Current user's working directory.
  std::string current_working_dir_;

  /// Mapping from FTP command to the corresponding handler function.
  const std::unordered_map<protocol::ftp::request::FtpCommand, FtpHandler>
      ftp_handlers_;

  // ------------------ HTTP ------------------
  /// Mapping from HTTP method to the corresponding handler function.
  const std::unordered_map<protocol::http::request::HttpMethod, HttpHandler>
      http_handlers_;
};
}  // namespace object_storage
}  // namespace server

#endif  // SERVER_OBJECT_STORAGE_SRC_SESSION_HPP
