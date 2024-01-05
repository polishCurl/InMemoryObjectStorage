#include "utils/src/utils.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace utils;

TEST(SplitString, Empty) {
  std::string str{""};
  ASSERT_TRUE(split(str, " ").empty());
}

TEST(SplitString, OneToken) {
  std::string str{"Token"};
  ASSERT_THAT(split(str, " "), ::testing::ElementsAreArray({"Token"}));
}

TEST(SplitString, FourTokensThreeEmpty) {
  std::string str{" 1  "};
  ASSERT_THAT(split(str, " "), ::testing::ElementsAreArray({"", "1", "", ""}));
}

TEST(SplitString, DashDelimiter) {
  std::string str{"I-like-TRAINS"};
  ASSERT_THAT(split(str, "-"),
              ::testing::ElementsAreArray({"I", "like", "TRAINS"}));
}

TEST(SplitString, Http) {
  std::string str{
      "GET /index.html HTTP/1.1\r\n"
      "\r\n"};
  ASSERT_THAT(split(str, "\r\n"), ::testing::ElementsAreArray(
                                      {"GET /index.html HTTP/1.1", "", ""}));
}