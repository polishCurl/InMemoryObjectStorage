#include "integration_tests.hpp"

using namespace server::object_storage;
using namespace test;
using namespace test::http;

TEST_P(IntegrationTest, StartStop) {}

TEST_P(IntegrationTest, Unsupported) {
  ASSERT_EQ(400, curl("/", "HEAD", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(IntegrationTest, ListEmpty) {
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(IntegrationTest, RemoveFromEmpty) {
  ASSERT_EQ(404, curl("/does/not/exists.txt", "DELETE", authenticate_));
}

TEST_P(IntegrationTest, UploadDownload) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/diff_name_th!!_uS**al.jpeg");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(uri.size() + 1, std::filesystem::file_size(kOutFileName));
  ASSERT_EQ(200, curl(uri, "GET", authenticate_));
  ASSERT_TRUE(compareFiles(file_to_upload, std::string{kOutFileName}));
}

TEST_P(IntegrationTest, UploadRemove) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/data/example.json");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(uri.size() + 1, std::filesystem::file_size(kOutFileName));
  ASSERT_EQ(200, curl(uri, "DELETE", authenticate_));
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(IntegrationTest, UploadTwice) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/test/data/example.json");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(404, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(uri.size() + 1, std::filesystem::file_size(kOutFileName));
}

TEST_P(IntegrationTest, DeleteTwice) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/test/data/example.json");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(200, curl(uri, "DELETE", authenticate_));
  ASSERT_EQ(404, curl(uri, "DELETE", authenticate_));
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(IntegrationTest, GetFileNotFound) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri_upload("/test/data/example.json");
  const std::string uri_download("/example.json");
  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri_upload, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(404, curl(uri_download, "GET", authenticate_));
}

TEST_P(IntegrationTest, MultipleLargeFiles) {
  std::vector<std::string> files{
      "test/data/the_office_theme.mp3",
      "test/data/bmw_picture.jpeg",
      "test/data/household_expenditure.csv",
  };

  for (const auto& file : files) {
    ASSERT_TRUE(std::filesystem::exists(file));
    ASSERT_EQ(201, curl("/" + file, "PUT", authenticate_, file));
  }

  for (const auto& file : files) {
    ASSERT_EQ(200, curl("/" + file, "GET", authenticate_));
    ASSERT_TRUE(compareFiles(file, std::string{kOutFileName}));
  }
}

TEST_P(IntegrationTest, NotAuthorized) {
  const std::string username{"Lando"};
  const std::string password{"Norris"};
  const std::string file("test/data/example.json");
  const std::string uri("/test/data/example.json");
  ASSERT_TRUE(std::filesystem::exists(file));

  int http_status = std::get<1>(GetParam()) ? 401 : 200;
  int http_status_put = std::get<1>(GetParam()) ? 401 : 201;
  ASSERT_EQ(http_status_put,
            curl(uri, "PUT", authenticate_, file, username, password));
  ASSERT_EQ(http_status, curl("/", "GET", authenticate_,
                              std::string{kOutFileName}, username, password));
  ASSERT_EQ(http_status, curl(uri, "GET", authenticate_,
                              std::string{kOutFileName}, username, password));
  ASSERT_EQ(http_status, curl(uri, "DELETE", authenticate_,
                              std::string{kOutFileName}, username, password));
}

/**
 * \brief Instantiate parametrized ObjectStorage HTTP test suite
 *
 * Parameters tested:
 * 1. Thread count used by the ObjectStorage server.
 * 2. User authentication disabled/enabled.
 */
INSTANTIATE_TEST_SUITE_P(Http, IntegrationTest,
                         ::testing::Values(TestParams{1, false},
                                           TestParams{8, false},
                                           TestParams{1, true},
                                           TestParams{3, true}));