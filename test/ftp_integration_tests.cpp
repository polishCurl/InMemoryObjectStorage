#include "integration_tests.hpp"

using namespace server::object_storage;
using namespace test;
using namespace test::ftp;

TEST_P(IntegrationTest, ListEmpty) {
  ASSERT_EQ(0, curl("", {}, authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}
/*
TEST_P(IntegrationTest, RemoveFromEmpty) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/test/data/example.json");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, http::curl(uri, "PUT", authenticate_, file_to_upload));

  ASSERT_TRUE(curl("/test/data/example.json",
                   std::make_optional<std::string>("DELE"), authenticate_));
}
*/

TEST_P(IntegrationTest, RemoveFromEmpty) {
  ASSERT_EQ(550, curl("/test/data/example.json",
                      std::make_optional<std::string>("DELE"), authenticate_));
}

TEST_P(IntegrationTest, NotAuthorized) {
  const std::string username{"Lando"};
  const std::string password{"Norris"};
  const std::string file("test/data/example.json");
  const std::string uri("/test/data/example.json");

  int ftp_reply_code = std::get<1>(GetParam()) ? 530 : 550;
  ASSERT_EQ(ftp_reply_code, curl(uri, std::make_optional<std::string>("DELE"),
                                 authenticate_, file, username, password));
}

/**
 * \brief Instantiate parametrized ObjectStorage FTP test suite
 *
 * Parameters tested:
 * 1. Thread count used by the ObjectStorage server.
 * 2. User authentication disabled/enabled.
 */
INSTANTIATE_TEST_SUITE_P(Ftp, IntegrationTest,
                         ::testing::Values(TestParams{1, false},
                                           TestParams{8, false},
                                           TestParams{1, true},
                                           TestParams{3, true}));
