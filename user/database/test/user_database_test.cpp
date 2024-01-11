#include "user/database/src/user_database.hpp"

#include <sstream>

#include "gtest/gtest.h"

using namespace user;

TEST(UserDatabaseTest, InitialState) {
  UserDatabase db{};
  EXPECT_FALSE(db.add({"anonymous", "admin4321"}));
  EXPECT_TRUE(db.verify({"anonymous", "admin4321"}));
  EXPECT_TRUE(db.verify({"anonymous", "!"}));
  EXPECT_FALSE(db.verify({"NORD", "1234567890!"}));
}

TEST(UserDatabaseTest, AddUser) {
  UserDatabase db{};
  EXPECT_TRUE(db.add({"BMW", "m340i"}));
  EXPECT_TRUE(db.verify({"BMW", "m340i"}));
  EXPECT_FALSE(db.add({"BMW", "m340i"}));
  EXPECT_TRUE(db.verify({"BMW", "m340i"}));
  EXPECT_FALSE(db.add({"BMW", "M3"}));
  EXPECT_TRUE(db.add({"Audi", "m340i"}));
}

TEST(UserTest, StreamInsertion) {
  User user{"Jan Pawel", "1670"};
  std::stringstream ss;
  ss << user;
  EXPECT_EQ(ss.str(), "Jan Pawel:1670");
}
