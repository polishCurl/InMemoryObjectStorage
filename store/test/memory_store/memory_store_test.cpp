#include "store/src/memory_store/memory_store.hpp"

#include "gtest/gtest.h"

using namespace store;

TEST(MemoryStorePut, Success) {
  MemoryStore ms;
  File file{10, static_cast<char>(0xed)};
  ASSERT_EQ(Result::kSuccess, ms.put("test_file.txt", file));
  File another_file{1, static_cast<char>(0x11)};
  EXPECT_EQ(Result::kSuccess, ms.put("path.test_file.txt", another_file));
}

TEST(MemoryStorePut, AlreadyExists) {
  MemoryStore ms;
  File file{10, static_cast<char>(0xed)};
  ASSERT_EQ(Result::kSuccess, ms.put("a.out", file));
  File another_file{1, static_cast<char>(0x11)};
  EXPECT_EQ(Result::kAlreadyExists, ms.put("a.out", another_file));
}

TEST(MemoryStoreGet, FileNotFound) {
  MemoryStore ms;
  ASSERT_EQ(Result::kFileNotFound, ms.get("some/path/to/file").first);

  File file{3, 'z'};
  ASSERT_EQ(Result::kSuccess, ms.put("some/path/to/file_other", file));
  ASSERT_EQ(Result::kFileNotFound, ms.get("some/path/to/file").first);
}

TEST(MemoryStoreGet, Success) {
  MemoryStore ms;
  File file{3, 'z'};
  ASSERT_EQ(Result::kSuccess, ms.put("/tmp/temp.txt", file));
  const auto retrieved_file = ms.get("/tmp/temp.txt");
  ASSERT_EQ(Result::kSuccess, retrieved_file.first);
  ASSERT_EQ(file, retrieved_file.second);
}

TEST(MemoryStoreGet, SameFileMultipleTimes) {
  MemoryStore ms;
  File file{3, 'z'};
  ASSERT_EQ(Result::kSuccess, ms.put("/tmp/temp.txt", file));

  for (int i = 0; i < 3; i++) {
    const auto retrieved_file = ms.get("/tmp/temp.txt");
    ASSERT_EQ(Result::kSuccess, retrieved_file.first);
    ASSERT_EQ(file, retrieved_file.second);
  }
}

TEST(MemoryStoreList, Empty) {
  MemoryStore ms;
  ASSERT_EQ(0, ms.list().size());
}

TEST(MemoryStoreList, Success) {
  MemoryStore ms;
  File file{1, static_cast<char>(0x86)};
  ASSERT_EQ(Result::kSuccess, ms.put("b.hpp", file));
  ASSERT_EQ(FileList{"b.hpp"}, ms.list());

  ASSERT_EQ(Result::kSuccess, ms.put("a.hpp", file));
  auto file_list = ms.list();
  std::sort(file_list.begin(), file_list.end());
  ASSERT_EQ(FileList({"a.hpp", "b.hpp"}), file_list);
}

TEST(MemoryStoreRemove, FileNotFound) {
  MemoryStore ms;
  ASSERT_EQ(Result::kFileNotFound, ms.remove("some/path/to/file"));
}

TEST(MemoryStoreRemove, Success) {
  MemoryStore ms;
  File file{3, 'z'};
  ASSERT_EQ(Result::kSuccess, ms.put("/tmp/temp.txt", file));
  ASSERT_EQ(Result::kSuccess, ms.remove("/tmp/temp.txt"));
  ASSERT_EQ(Result::kFileNotFound, ms.remove("/tmp/temp.txt"));
}
