#include "server/object_storage/src/object_storage.hpp"

#include "gtest/gtest.h"

using namespace server::object_storage;

TEST(ObjectStorageTest, ConstructorDefault) {
  ObjectStorage server{};
  EXPECT_EQ("0.0.0.0", server.getAddress());
  EXPECT_EQ(21, server.getPort());
}

TEST(ObjectStorageTest, ConstructorSetPortAndAddress) {
  ObjectStorage server{"127.0.0.1", 80};
  EXPECT_EQ("127.0.0.1", server.getAddress());
  EXPECT_EQ(80, server.getPort());
}

TEST(ObjectStorageTest, AddUser) {
  ObjectStorage server{"1.2.3.4", 120};
  EXPECT_FALSE(server.addUser("anonymous", "admin4321"));
  ASSERT_TRUE(server.addUser("Locker", "!vilnus*"));
  EXPECT_FALSE(server.addUser("Locker", "DMVEJRFN28U123Y"));
  EXPECT_TRUE(server.addUser("Nord", "THREAD_proTeCTED"));
}

TEST(ObjectStorageTest, Start) {
  ObjectStorage server{};
  EXPECT_TRUE(server.start());
}
