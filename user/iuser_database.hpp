#ifndef USER_IUSER_DATABASE_HPP
#define USER_IUSER_DATABASE_HPP

#include <iostream>
#include <string>

namespace user {

/**
 * \brief User representation.
 */
struct User {
  std::string username;  ///< Username.
  std::string password;  ///< Password used to authenticate user.
};

/**
 * \brief Stream insertion operator for User class.
 *
 * \param os Output stream.
 * \param user User to insert into the stream.
 *
 * \return Updated output stream.
 */
inline std::ostream& operator<<(std::ostream& os, const User& user) {
  os << user.username << ':' << user.password;
  return os;
}

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
  [[nodiscard]] virtual bool verify(const User& user) const = 0;
};

}  // namespace user

#endif  // USER_IUSER_DATABASE_HPP
