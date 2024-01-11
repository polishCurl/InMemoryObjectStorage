#ifndef SERVER_ISERVER_HPP
#define SERVER_ISERVER_HPP

#include <string>

namespace server {

/**
 * \brief Server interface.
 */
class IServer {
 public:
  virtual ~IServer() = default;

  /**
   * \brief Start server.
   *
   * \param thread_count Number of threads to use.
   *
   * \return True if server was started successfully, false otherwise.
   */
  virtual bool start(std::size_t thread_count) = 0;

  /**
   * \brief Get the port on which server is listening.
   *
   * \return The port number.
   */
  virtual std::uint16_t getPort() const noexcept = 0;

  /**
   * \brief Get the IP address the server is listening on.
   *
   * \return The IP address.
   */
  virtual std::string getAddress() const noexcept = 0;

  /**
   * \brief Add a new user with the given username and password.
   *
   * \param username The username for login.
   * \param password The user's password.
   *
   * \return True if user was added successfully, false otherwise.
   */
  virtual bool addUser(const std::string& username,
                       const std::string& password) = 0;
};

}  // namespace server

#endif  // SERVER_ISERVER_HPP
