#ifndef USER_IUSER_DATABASE_HPP
#define USER_IUSER_DATABASE_HPP

#include <string>

namespace user {

/**
 * \brief User representation.
 */
struct User {
  std::string username;  ///< Username.
  std::string password;  ///< Password used to authenticate user.

  bool operator==(const User& other) const {
    return (username == other.username) && (password == other.password);
  }
};

/**
 * \brief User database interface.
 */
class IUserDatabase {
 public:
  virtual ~IUserDatabase() = default;

  /**
   * \brief Add a new user with the given username and password.
   *
   * \param user User to add.
   *
   * \return True if user was added successfully, false otherwise.
   */
  virtual bool add(const User& user) = 0;

  /**
   * \brief Check if a user with the given credentials exists in the database.
   *
   * \param user User to check.
   *
   * \return True if user exists in the database, false otherwise.
   */
  virtual bool exists(const User& user) const = 0;
};

}  // namespace user

#endif  // USER_IUSER_DATABASE_HPP
