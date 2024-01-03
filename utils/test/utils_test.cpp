#include "utils/src/utils.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace utils;

TEST(SplitString, Empty) {
  std::string str{""};
  ASSERT_TRUE(split(str, ' ').empty());
}

TEST(SplitString, OneToken) {
  std::string str{"Token"};
  ASSERT_THAT(split(str, ' '), ::testing::ElementsAreArray({"Token"}));
}

TEST(SplitString, FourTokensThreeEmpty) {
  std::string str{" 1  "};
  ASSERT_THAT(split(str, ' '), ::testing::ElementsAreArray({"", "1", "", ""}));
}

TEST(SplitString, DashDelimiter) {
  std::string str{"I-like-TRAINS"};
  ASSERT_THAT(split(str, '-'),
              ::testing::ElementsAreArray({"I", "like", "TRAINS"}));
}
