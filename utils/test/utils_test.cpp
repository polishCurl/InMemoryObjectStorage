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

TEST(ToUpperCase, Empty) {
  std::string str{};
  toUpperCase(str);
  EXPECT_EQ("", str);
}

TEST(ToUpperCase, AlreadyUpperCase) {
  std::string str{"ABCDEF"};
  toUpperCase(str);
  EXPECT_EQ("ABCDEF", str);
}

TEST(ToUpperCase, Simple) {
  std::string str{"abcdef"};
  toUpperCase(str);
  EXPECT_EQ("ABCDEF", str);
}

TEST(ToUpperCase, SpecialChars) {
  std::string str{"_A1Be*Z/ "};
  toUpperCase(str);
  EXPECT_EQ("_A1BE*Z/ ", str);
}

TEST(ToLowerCase, Empty) {
  std::string str{};
  toLowerCase(str);
  EXPECT_EQ("", str);
}

TEST(ToLowerCase, AlreadyLowerCase) {
  std::string str{"abcdef"};
  toLowerCase(str);
  EXPECT_EQ("abcdef", str);
}

TEST(ToLowerCase, Simple) {
  std::string str{"ABCDEF"};
  toLowerCase(str);
  EXPECT_EQ("abcdef", str);
}

TEST(ToLowerCase, SpecialChars) {
  std::string str{"_A1Be*Z/ "};
  toLowerCase(str);
  EXPECT_EQ("_a1be*z/ ", str);
}