#include "integration_tests.hpp"

using namespace server::object_storage;
using namespace test;
using namespace test::ftp;

TEST_P(IntegrationTest, Unsupported) {
  ASSERT_EQ(500, curl(TestScenario::NotSupported, "/", authenticate_));
}

TEST_P(IntegrationTest, ListEmpty) {
  ASSERT_EQ(0, curl(TestScenario::List, "", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(IntegrationTest, RemoveFromEmpty) {
  ASSERT_EQ(550,
            curl(TestScenario::Dele, "/test/data/example.json", authenticate_));
}

TEST_P(IntegrationTest, UploadRemove) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/data/example.json");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(0, curl(TestScenario::Stor, uri, authenticate_, file_to_upload));
  ASSERT_EQ(0, curl(TestScenario::List, "", authenticate_));
  ASSERT_EQ(uri.size() + 1, std::filesystem::file_size(kOutFileName));
  ASSERT_EQ(0, curl(TestScenario::Dele, uri, authenticate_));
  ASSERT_EQ(0, curl(TestScenario::List, "", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(IntegrationTest, UploadTwice) {
  const std::string file_to_upload("test/data/bmw_picture.jpeg");
  const std::string uri("/BMW/picture.jpeg");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(0, curl(TestScenario::Stor, uri, authenticate_, file_to_upload));
  ASSERT_EQ(450, curl(TestScenario::Stor, uri, authenticate_, file_to_upload));
  ASSERT_EQ(0, curl(TestScenario::List, "", authenticate_));
  ASSERT_EQ(uri.size() + 1, std::filesystem::file_size(kOutFileName));
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

TEST_P(IntegrationTest, NotAuthorized) {
  const std::string username{"Lando"};
  const std::string password{"Norris"};
  const std::string file("test/data/example.json");
  const std::string uri("/test/data/example.json");

  const auto authenticate = std::get<1>(GetParam());

  int ftp_reply_code = authenticate ? 530 : 0;
  ASSERT_EQ(0, curl(TestScenario::List, "", authenticate_));

  ftp_reply_code = authenticate ? 530 : 550;
  ASSERT_EQ(ftp_reply_code, curl(TestScenario::Dele, uri, authenticate_, file,
                                 username, password));
  ftp_reply_code = authenticate ? 530 : 0;
  ASSERT_EQ(ftp_reply_code, curl(TestScenario::Stor, uri, authenticate_, file,
                                 username, password));
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
