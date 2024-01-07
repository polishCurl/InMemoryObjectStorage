#ifndef SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP
#define SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP

#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <thread>

#include "filesystem/memory_fs/src/memory_fs.hpp"
#include "server/iserver.hpp"
#include "user/database/src/user_database.hpp"

namespace server {

namespace object_storage {

/**
 * \brief Object storage server logging level.
 */
enum class LogLevel : int { trace = 0, debug, info, warning, error, fatal };

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
   * \note The default port number 21 requires the application to have root
   * privileges.
   *
   * \param address The host to accept incoming connections from.
   * \param port The port to start the server on.
   * \param log_level Logging level used by the server (verbosity).
   */
  ObjectStorage(const std::string& address = std::string("0.0.0.0"),
                uint16_t port = 21, LogLevel log_level = LogLevel::info);

  // No use case for copying and moving for now.
  ObjectStorage(ObjectStorage&&) = delete;
  ObjectStorage& operator=(ObjectStorage&&) = delete;
  ObjectStorage(const ObjectStorage&) = delete;
  ObjectStorage& operator=(const ObjectStorage&) = delete;

  ~ObjectStorage() { stop(); };

  bool start(std::size_t thread_count = 1) override;
  void stop() override;

  inline std::uint16_t getPort() const noexcept override { return port_; };
  inline std::string getAddress() const noexcept override { return address_; }
  bool addUser(const std::string& username,
               const std::string& password) noexcept override;

 protected:
  user::UserDatabase users_;  ///< Users recognised by the server
  fs::MemoryFs filesystem_;   ///< In-memory file storage
  std::string address_;       ///< Host on which to accept connections
  const uint16_t port_;       ///< Server port number
  LogLevel log_level_;        ///< Server logging level

  std::vector<std::thread> thread_pool_;     ///< Server worker threads
  boost::asio::io_service io_service_;       ///< OS IO services
  boost::asio::ip::tcp::acceptor acceptor_;  ///< TCP connections acceptor
  std::atomic<int> open_connection_count_;   ///< Open TCP connection count

 private:
  bool setUpAcceptor() noexcept;
  void setUpLogging() noexcept;
};

}  // namespace object_storage
}  // namespace server

#endif  // SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP
