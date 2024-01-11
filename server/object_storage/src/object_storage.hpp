#ifndef SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP
#define SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP

#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <thread>

#include "filesystem/memory_fs/src/memory_fs.hpp"
#include "server/iserver.hpp"
#include "session.hpp"
#include "user/database/src/user_database.hpp"

namespace server {

namespace object_storage {

using ThreadPool = std::vector<std::thread>;

/**
 * \brief Object storage server logging level.
 */
enum class LogLevel : int { Trace = 0, Debug, Info, Warning, Error, Eatal };

/**
 * \brief Object storage server implementation.
 *
 * Object storage is an FTP/HTTP server for in-memory object storage and
 * retrieval.
 */
class ObjectStorage : public IServer {
 public:
  /**
   * \brief Create an object storage server instance that will listen for HTTP
   * and FTP requests on the given port and accept connections from the given
   * network interface.
   *
   * \attention The object storage constructor accepts FTP port range as an
   * argument. This argument is use to distinguish between HTTP and FTP clients
   * establishing connection with this server. All client ports outside the FTP
   * range are assumed HTTP.
   *
   * \param address IPv4 address.
   * \param port On which the server will be listening for new connections.
   * \param log_level Logging level used by the server (logging verbosity).
   * \param authenticate Enable/disable user authentication.
   * \param ftp_port_range Client port numbers to use for FTP (inclusive range).
   */
  ObjectStorage(const std::string& address = std::string("0.0.0.0"),
                uint16_t port = 21, LogLevel log_level = LogLevel::Info,
                bool authenticate = false,
                PortRange ftp_port_range = {2000, 3000});

  // No use case for copying and moving for now.
  ObjectStorage(ObjectStorage&&) = delete;
  ObjectStorage& operator=(ObjectStorage&&) = delete;
  ObjectStorage(const ObjectStorage&) = delete;
  ObjectStorage& operator=(const ObjectStorage&) = delete;

  ~ObjectStorage() { stop(); };

  bool start(std::size_t thread_count = 1) override;

  inline std::uint16_t getPort() const noexcept override { return port_; };
  inline std::string getAddress() const noexcept override { return address_; }
  bool addUser(const std::string& username,
               const std::string& password) noexcept override;

 private:
  /**
   * \brief Stop server.
   */
  void stop();

  /**
   * \brief Set up TCP connection acceptor on HTTP/FTP socket.
   *
   * \return True if server is ready to accept connections, false otherwise.
   */
  bool configureAcceptor() noexcept;

  /**
   * \brief Accept connection request from client on the HTTP/FTP command
   * socket.
   *
   * \note This method is asynchronous.
   *
   * \param session Session created for the connection.
   * \param error_code Error code.
   */
  void acceptConnection(const std::shared_ptr<Session>& session,
                        ErrorCode const& error_code) noexcept;

  /**
   * \brief Set up server logging.
   */
  void setUpLogging() noexcept;

  user::UserDatabase users_;  ///< Server users
  fs::MemoryFs filesystem_;   ///< In-memory file storage
  std::string address_;       ///< IPv4 addres used by the server
  const uint16_t port_;       ///< Server port number
  LogLevel log_level_;        ///< Server logging level

  ThreadPool workers_;        ///< Server worker threads
  IOService io_service_;      ///< OS IO services
  Acceptor acceptor_;         ///< TCP connection acceptor
  const bool authenticate_;   ///< Authenticate users
  PortRange ftp_port_range_;  ///< Port numbers to use for FTP.
};

}  // namespace object_storage
}  // namespace server

#endif  // SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP
