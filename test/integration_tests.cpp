#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <string>
#include <tuple>

#include "gtest/gtest.h"
#include "server/object_storage/src/object_storage.hpp"

using namespace server::object_storage;

/// Scratch file to use for testing
static constexpr std::string_view kOutFileName{"/tmp/object_store_out"};

/// Scratch file to store HTTP status of last request
/*static constexpr std::string_view
 * kStatusFileName{"/tmp/object_store_status"};*/

/// Default username used for authentication.
static constexpr std::string_view kUsername{"object_store"};

/// Default password used for authentication.
static constexpr std::string_view kPassword{"admin4321"};

/// Default hostname used for testing.
static constexpr std::string_view kHostname{"127.0.0.1"};

/// Default server port number used for testing.
static constexpr std::uint16_t kServerPortId{1670};

/// Default minimum client port number used for HTTP.
static constexpr std::uint16_t kMinHttpClientPortId{50000};

/// Default maximum client port number used for HTTP.
static constexpr std::uint16_t kMaxHttpClientPortId{51000};

/// Default client port number used for HTTP.
// static constexpr std::uint16_t kFtpClientPortId{2147};

/// Default log level used for the server.
static constexpr LogLevel kServerLogLevel{LogLevel::Error};

/// Testing parameters: Server thread count and enable/disable user
/// authentication.
using TestParams = std::tuple<std::size_t, bool>;

/**
 * \brief Run 'curl' HTTP command with given parameters.
 *
 * \param uri Uniform Resource Identifier.
 * \param method HTTP method (use uppercase)
 * \param authenticate Use HTTP authentication, or not.
 * \param filename Local file to use for download/upload.
 * \param username Username to authenticate.
 * \param password Password to authenticate.
 * \param host Hostname to use.
 * \param port Port ID to use.
 *
 * \return HTTP status code returned by the server.
 *
 * \throw std::runtime_error If curl command fails to execute.
 */
static int curl(const std::string& uri, const std::string& method,
                bool authenticate = false,
                const std::string& filename = std::string{kOutFileName},
                const std::string& username = std::string{kUsername},
                const std::string& password = std::string{kPassword},
                const std::string& host = std::string{kHostname},
                std::uint16_t port = kServerPortId)

{
  static constexpr std::array<std::string_view, 2> kHttpDownloadMethods{"GET",
                                                                        "HEAD"};

  static constexpr std::array<std::string_view, 2> kHttpUploadMethods{"PUT",
                                                                      "POST"};

  std::string command{"curl -s -S"};
  command += " http://" + host + ':' + std::to_string(port) + uri;
  command += " -X " + method;

  // If HTTP method is for downloading a file, provide the output file.
  // Otherwise, if HTTP method is for uploading file, provide the input file.
  if (std::find(kHttpDownloadMethods.begin(), kHttpDownloadMethods.end(),
                method) != kHttpDownloadMethods.end()) {
    command += " -o " + filename;
  } else if (std::find(kHttpUploadMethods.begin(), kHttpUploadMethods.end(),
                       method) != kHttpUploadMethods.end()) {
    command += " -T " + filename;
  }

  // Provide user credentials
  if (authenticate) {
    command += " --user \"" + username + ':' + password + '\"';
  }

  // Provide the port number range to use for HTTP
  command += "  --local-port " + std::to_string(kMinHttpClientPortId) + '-' +
             std::to_string(kMaxHttpClientPortId);

  // Make curl output only HTTP status code returned by the server.
  command += " -w \"%{http_code}\n\" ";

  // Extract the HTTP status code from curl's stdout.
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                pclose);
  if (!pipe) {
    throw std::runtime_error("curl command failed failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
    result += buffer.data();
  }

  return std::stoi(result);
}

/**
 * \brief Check if two files are binary equal.
 *
 * \return True if files are equal, false otherwise.
 */
static bool compareFiles(const std::string& path1, const std::string& path2) {
  std::ifstream file1(path1, std::ifstream::binary | std::ifstream::ate);
  std::ifstream file2(path2, std::ifstream::binary | std::ifstream::ate);

  if (file1.fail() || file2.fail()) {
    return false;
  }

  if (file1.tellg() != file2.tellg()) {
    return false;
  }

  file1.seekg(0, std::ifstream::beg);
  file2.seekg(0, std::ifstream::beg);
  return std::equal(std::istreambuf_iterator<char>(file1.rdbuf()),
                    std::istreambuf_iterator<char>(),
                    std::istreambuf_iterator<char>(file2.rdbuf()));
}

/**
 * \brief ObjectStorage HTTP test fixture (parametrized)
 *
 */
class HttpFixture : public ::testing::TestWithParam<TestParams> {
 protected:
  /**
   * \brief Test setup.
   *
   * Create ObjectStorage fixture and set it up according to the test
   * parameters.
   */
  void SetUp() override {
    const auto [thread_count, authenticate] = GetParam();
    authenticate_ = authenticate;
    thread_count_ = thread_count;

    server_ = std::make_unique<ObjectStorage>(
        std::string{kHostname}, kServerPortId, kServerLogLevel, authenticate_);
    EXPECT_TRUE(
        server_->addUser(std::string{kUsername}, std::string{kPassword}));
    ASSERT_TRUE(server_->start(thread_count_));
  }

  /**
   * \brief Test teardown.
   *
   * Delete local temporary file used only for testing.
   */
  void TearDown() override { std::filesystem::remove(kOutFileName); }

  std::unique_ptr<ObjectStorage> server_;
  bool authenticate_;
  std::size_t thread_count_;
};

TEST_P(HttpFixture, StartStop) {}

TEST_P(HttpFixture, UnrecognizedMethod) {
  ASSERT_EQ(400, curl("/", "HEAD", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(HttpFixture, ListEmpty) {
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(HttpFixture, RemoveFromEmpty) {
  ASSERT_EQ(404, curl("/does/not/exists.txt", "DELETE", authenticate_));
}

TEST_P(HttpFixture, UploadDownload) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/diff_name_th!!_uS**al.jpeg");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(uri.size() + 1, std::filesystem::file_size(kOutFileName));
  ASSERT_EQ(200, curl(uri, "GET", authenticate_));
  ASSERT_TRUE(compareFiles(file_to_upload, std::string{kOutFileName}));
}

TEST_P(HttpFixture, UploadRemove) {
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

TEST_P(HttpFixture, UploadTwice) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/test/data/example.json");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(404, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(uri.size() + 1, std::filesystem::file_size(kOutFileName));
}

TEST_P(HttpFixture, DeleteTwice) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri("/test/data/example.json");

  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(200, curl(uri, "DELETE", authenticate_));
  ASSERT_EQ(404, curl(uri, "DELETE", authenticate_));
  ASSERT_EQ(200, curl("/", "GET", authenticate_));
  ASSERT_EQ(0, std::filesystem::file_size(kOutFileName));
}

TEST_P(HttpFixture, GetFileNotFound) {
  const std::string file_to_upload("test/data/example.json");
  const std::string uri_upload("/test/data/example.json");
  const std::string uri_download("/example.json");
  ASSERT_TRUE(std::filesystem::exists(file_to_upload));
  ASSERT_EQ(201, curl(uri_upload, "PUT", authenticate_, file_to_upload));
  ASSERT_EQ(404, curl(uri_download, "GET", authenticate_));
}

TEST_P(HttpFixture, MultipleLargeFiles) {
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

TEST_P(HttpFixture, NotAuthorized) {
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
 * \brief Parametrized ObjectStorage test suite
 *
 * Parameters tested:
 * 1. Thread count used by the ObjectStorage server.
 * 2. User authentication disabled/enabled.
 */
INSTANTIATE_TEST_SUITE_P(Integration, HttpFixture,
                         ::testing::Values(TestParams{1, false},
                                           TestParams{8, false},
                                           TestParams{1, true},
                                           TestParams{3, true}));
