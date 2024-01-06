#include "user_database.hpp"

namespace user {

bool UserDatabase::add(const User& user) {
  if (!requirePassword(user)) {
    return false;
  }
  std::unique_lock lock(mutex_);
  return users_.insert({user.username, user.password}).second;
}

bool UserDatabase::exists(const User& user) const {
  if (!requirePassword(user)) {
    return true;
  }
  std::shared_lock lock(mutex_);
  return (users_.find(user.username) != users_.end()) &&
         (users_.at(user.username) == user.password);
}

bool UserDatabase::requirePassword(const User& user) const {
  return kUsersWithoutPassword.find(user.username) ==
         kUsersWithoutPassword.end();
}

const std::unordered_set<std::string> UserDatabase::kUsersWithoutPassword{
    "anonymous"};

}  // namespace user
