#ifndef TEST_INTEGRATION_TESTS_HPP
#define TEST_INTEGRATION_TESTS_HPP

#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>

#include "gtest/gtest.h"
#include "server/object_storage/src/object_storage.hpp"

namespace test {

/// Scratch file to use for testing
static constexpr std::string_view kOutFileName{"/tmp/object_store_out"};

/// Default username used for authentication.
static constexpr std::string_view kUsername{"object_store"};

/// Default password used for authentication.
static constexpr std::string_view kPassword{"admin4321"};

/// Default hostname used for testing.
static constexpr std::string_view kHostname{"127.0.0.1"};

/// Default server port number used for testing.
static constexpr std::uint16_t kServerPortId{1670};

/// Default log level used for the server.
static constexpr server::object_storage::LogLevel kServerLogLevel{
    server::object_storage::LogLevel::Error};

/// Testing parameters: Server thread count and enable/disable user
/// authentication.
using TestParams = std::tuple<std::size_t, bool>;

/**
 * \brief Check if two files are binary equal.
 *
 * \param path1 Path to the first file to compare.
 * \param path2 Path to the second file to compare.
 *
 * \return True if files are equal, false otherwise.
 */
bool compareFiles(const std::string& path1, const std::string& path2) {
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
 * \brief Execute the command provided and capture stdout.
 *
 * \throw std::runtime_error If curl command fails to execute.
 *
 * \param command Command to execute.
 *
 * \return stdout from the executed command.
 */
std::string execute(const std::string command) {
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

  return result;
}

/**
 * \brief ObjectStorage integration test fixture (parametrized)
 */
class IntegrationTest : public ::testing::TestWithParam<TestParams> {
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

    server_ = std::make_unique<server::object_storage::ObjectStorage>(
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
  void TearDown() override { /* std::filesystem::remove(kOutFileName); */
  }

  /// Object storage server under test.
  std::unique_ptr<server::object_storage::ObjectStorage> server_;
  bool authenticate_;         ///< Enable/disable user server authentication
  std::size_t thread_count_;  ///< Number of threads used by the server.
};

/////////////////////////////////////////////////////////////////////////
//  HTTP
/////////////////////////////////////////////////////////////////////////
namespace http {

/// Default minimum client port number used for HTTP.
static constexpr std::uint16_t kMinHttpClientPortId{50000};

/// Default maximum client port number used for HTTP.
static constexpr std::uint16_t kMaxHttpClientPortId{51000};

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
 */
int curl(const std::string& uri, const std::string& method,
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

  return std::stoi(execute(command));
}
}  // namespace http

/////////////////////////////////////////////////////////////////////////
//  FTP
/////////////////////////////////////////////////////////////////////////
namespace ftp {

/// Default minimum client port number used for FTP.
static constexpr std::uint16_t kMinFtpClientPortId{2000};

/// Default maximum client port number used for FTP.
static constexpr std::uint16_t kMaxFtpClientPortId{3000};

/// FTP test scenarios
enum class TestScenario {
  List,
  Retr,
  Stor,
  Dele,
  NotSupported,
  ParamMissing,
};

/**
 * \brief Run 'curl' FTP command with given parameters.
 *
 * \param scenario Test scenario.
 * \param uri Uniform Resource Identifier.
 * \param authenticate Use FTP login, or not.
 * \param filename Local file to use for download/upload.
 * \param username Username to authenticate.
 * \param password Password to authenticate.
 * \param host Hostname to use.
 * \param port Port ID to use.
 *
 * \return 0 if curl completed successfully, FTP reply code otherwise.
 */
int curl(TestScenario scenario, const std::string& uri,
         bool authenticate = false,
         const std::string& filename = std::string{kOutFileName},
         const std::string& username = std::string{kUsername},
         const std::string& password = std::string{kPassword},
         const std::string& host = std::string{kHostname},
         std::uint16_t port = kServerPortId)

{
  std::string command{"curl -s -S"};
  command += " ftp://" + host + ':' + std::to_string(port);

  switch (scenario) {
    case TestScenario::List:
      command += " -o " + filename;
      break;
    case TestScenario::Dele:
      command += " -Q \"DELE " + uri + '\"';
      break;
    case TestScenario::NotSupported:
      command += " -Q \"REIN\"";
      break;
    case TestScenario::ParamMissing:
      command += " -Q \"DELE\"";
      break;
    case TestScenario::Stor:
      command += uri;
      command += " -T " + filename;
      break;
    case TestScenario::Retr:
      command += uri + " -o " + filename;
      break;
    default:
      break;
  }

  // Provide user credentials
  if (authenticate) {
    command += " --user \"" + username + ':' + password + '\"';
  }

  // Provide the port number range to use for HTTP
  command += "  --local-port " + std::to_string(kMinFtpClientPortId) + '-' +
             std::to_string(kMaxFtpClientPortId);

  command += " 2> /dev/stdout";

  const auto output = execute(command);
  if (output.size() == 0) {
    return 0;
  }

  auto split = [](std::string s, char del) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string token;
    while (!ss.eof()) {
      std::getline(ss, token, del);
      tokens.push_back(token);
    }
    return tokens;
  };

  const auto tokens = split(output, ' ');
  return std::stoi(tokens[tokens.size() - 1]);
}

}  // namespace ftp
}  // namespace test

#endif  // TEST_INTEGRATION_TESTS_HPP
