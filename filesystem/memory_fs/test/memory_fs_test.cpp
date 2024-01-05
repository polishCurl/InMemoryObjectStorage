#include "filesystem/memory_fs/src/memory_fs.hpp"

#include "gtest/gtest.h"

using namespace fs;

TEST(MemoryFsPut, Success) {
  MemoryFs ms;
  File file{10, static_cast<char>(0xed)};
  ASSERT_EQ(Result::Success, ms.add("test_file.txt", file));
  File another_file{1, static_cast<char>(0x11)};
  EXPECT_EQ(Result::Success, ms.add("path.test_file.txt", another_file));
}

TEST(MemoryFsPut, AlreadyExists) {
  MemoryFs ms;
  File file{10, static_cast<char>(0xed)};
  ASSERT_EQ(Result::Success, ms.add("a.out", file));
  File another_file{1, static_cast<char>(0x11)};
  EXPECT_EQ(Result::AlreadyExists, ms.add("a.out", another_file));
}

TEST(MemoryFsGet, FileNotFound) {
  MemoryFs ms;
  ASSERT_EQ(Result::FileNotFound, ms.get("some/path/to/file").first);

  File file{3, 'z'};
  ASSERT_EQ(Result::Success, ms.add("some/path/to/file_other", file));
  ASSERT_EQ(Result::FileNotFound, ms.get("some/path/to/file").first);
}

TEST(MemoryFsGet, Success) {
  MemoryFs ms;
  File file{"I like trains"};
  ASSERT_EQ(Result::Success, ms.add("/tmp/temp.txt", file));
  const auto retrieved_file = ms.get("/tmp/temp.txt");
  ASSERT_EQ(Result::Success, retrieved_file.first);
  ASSERT_EQ(file, retrieved_file.second);
}

TEST(MemoryFsGet, SameFileMultipleTimes) {
  MemoryFs ms;
  File file{3, 'z'};
  ASSERT_EQ(Result::Success, ms.add("/tmp/temp.txt", file));

  for (int i = 0; i < 3; i++) {
    const auto retrieved_file = ms.get("/tmp/temp.txt");
    ASSERT_EQ(Result::Success, retrieved_file.first);
    ASSERT_EQ(file, retrieved_file.second);
  }
}

TEST(MemoryFsList, Empty) {
  MemoryFs ms;
  ASSERT_EQ(0, ms.list().size());
}

TEST(MemoryFsList, Success) {
  MemoryFs ms;
  File file{1, static_cast<char>(0x86)};
  ASSERT_EQ(Result::Success, ms.add("b.hpp", file));
  ASSERT_EQ(FileList{"b.hpp"}, ms.list());

  ASSERT_EQ(Result::Success, ms.add("a.hpp", file));
  auto file_list = ms.list();
  std::sort(file_list.begin(), file_list.end());
  ASSERT_EQ(FileList({"a.hpp", "b.hpp"}), file_list);
}

TEST(MemoryFsRemove, FileNotFound) {
  MemoryFs ms;
  ASSERT_EQ(Result::FileNotFound, ms.remove("some/path/to/file"));
}

TEST(MemoryFsRemove, Success) {
  MemoryFs ms;
  File file{3, 'z'};
  ASSERT_EQ(Result::Success, ms.add("/tmp/temp.txt", file));
  ASSERT_EQ(Result::Success, ms.remove("/tmp/temp.txt"));
  ASSERT_EQ(Result::FileNotFound, ms.remove("/tmp/temp.txt"));
}
