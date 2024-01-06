#include "user/database/src/user_database.hpp"

#include "gtest/gtest.h"

using namespace user;

TEST(UserDatabaseTest, InitialState) {
  UserDatabase db{};
  EXPECT_FALSE(db.add({"anonymous", "admin4321"}));
  EXPECT_TRUE(db.exists({"anonymous", "admin4321"}));
  EXPECT_TRUE(db.exists({"anonymous", "!"}));
  EXPECT_FALSE(db.exists({"NORD", "1234567890!"}));
}

TEST(UserDatabaseTest, AddUser) {
  UserDatabase db{};
  EXPECT_TRUE(db.add({"BMW", "m340i"}));
  EXPECT_TRUE(db.exists({"BMW", "m340i"}));
  EXPECT_FALSE(db.add({"BMW", "m340i"}));
  EXPECT_TRUE(db.exists({"BMW", "m340i"}));
  EXPECT_FALSE(db.add({"BMW", "M3"}));
  EXPECT_TRUE(db.add({"Audi", "m340i"}));
}
