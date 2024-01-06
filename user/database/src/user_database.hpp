#ifndef USER_DATABASE_SRC_USER_DATABASE_HPP
#define USER_DATABASE_SRC_USER_DATABASE_HPP

#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

#include "user/iuser_database.hpp"

template <>
struct std::hash<user::User> {
  /**
   * \brief Hash function implementation for a User.
   *
   * \param user User to hash.
   */
  size_t operator()(const user::User& user) const {
    return std::hash<std::string>{}(user.username) ^
           (std::hash<std::string>{}(user.password) << 1);
  }
};

namespace user {

/**
 * \brief Thread-safe user database implementation.
 *
 * \note Username "anonymous" is reserved and does not require password. When
 * checking for "anonymous" user existence, the password field is ignored.
 */
class UserDatabase : public IUserDatabase {
 public:
  bool add(const User& user) override;
  bool exists(const User& user) const override;

 protected:
  bool requirePassword(const User& user) const;

  /// List of usernames not requiring password.
  static const std::unordered_set<std::string> kUsersWithoutPassword;

  /// List of users stored in the database.
  std::unordered_map<std::string, std::string> users_;

  /// Reader/Writer lock to allow mutiple threads to read the database,
  /// but only one thread to write to it.
  mutable std::shared_mutex mutex_;
};

}  // namespace user

#endif  // USER_DATABASE_SRC_USER_DATABASE_HPP
