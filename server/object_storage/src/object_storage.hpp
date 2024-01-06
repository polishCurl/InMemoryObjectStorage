#ifndef SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP
#define SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP

#include <atomic>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <thread>

#include "server/iserver.hpp"

namespace server {

namespace object_storage {

/**
 * \brief Object storage server implementation.
 *
 * Object storage is an FTP/HTTP server for in-memory object storage and
 * retrieval.
 */
class ObjectStorage : public IServer {
 public:
  /**
   * @brief Create an object storage server instance that will listen for HTTP
   * and FTP requests on the given port and accept connections from the given
   * network interface.
   *
   * \note The default port number 21 requires the application to have root
   * privileges.
   *
   * @param port The port to start the server on.
   * @param host The host to accept incoming connections from.
   */

  ObjectStorage(const std::string& address = std::string("0.0.0.0"),
                uint16_t port = 21);

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
  inline bool addUser(const std::string& username,
                      const std::string& password) override {
    return true;
  }

 protected:
  // user::UserDatabase users_;

  const std::string address_;
  const uint16_t port_;

  std::vector<std::thread> thread_pool_;
  boost::asio::io_service io_service_;
  boost::asio::ip::tcp::acceptor acceptor_;

  std::atomic<int> open_connection_count_;
};

}  // namespace object_storage
}  // namespace server

#endif  // SERVER_OBJECT_STORAGE_SRC_OBJECT_STORAGE_HPP
