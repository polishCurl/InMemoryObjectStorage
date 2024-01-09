#include "protocol/http/request/src/http_parser.hpp"

#include "gtest/gtest.h"

using namespace protocol::http::request;

TEST(HttpParserTest, InitialState) {
  const auto http = HttpParser("");
  EXPECT_FALSE(http.isValid());
  EXPECT_EQ(HttpMethod::Unrecognized, http.getMethod());
  EXPECT_TRUE(http.getUri().empty());
  EXPECT_EQ(http.getResourceSize(), 0);
  EXPECT_FALSE(http["content-lenght"]);
}

TEST(HttpParserTest, Put1) {
  std::string http_request{
      "PUT /xampp/tests/file/check.php HTTP/1.1\r\n"
      "Host: 127.0.0.1\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Length: 10\r\n"
      "Connection: close\r\n"
      "\r\n"
      "text1=sase"};

  HttpParser http{http_request};
  EXPECT_TRUE(http.isValid());
  EXPECT_EQ(HttpMethod::Put, http.getMethod());
  EXPECT_EQ(http.getUri(), "/xampp/tests/file/check.php");
  ASSERT_TRUE(http["content-length"]);
  EXPECT_EQ(http.getResourceSize(), sizeof("text1=sase") - 1);
  EXPECT_EQ(http["host"], "127.0.0.1");
}

TEST(HttpParserTest, Put2) {
  const std::string http_request{
      "PUT /test HTTP/1.1\r\n"
      "Host: www.myServer.com\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Length: 8\r\n"
      "Accept: */*\r\n"
      "\r\n"
      "someData"};

  HttpParser http{http_request};
  EXPECT_TRUE(http.isValid());
  EXPECT_EQ(HttpMethod::Put, http.getMethod());
  EXPECT_EQ(http.getUri(), "/test");
  ASSERT_EQ(http["content-length"], "8");
  EXPECT_EQ(http.getResourceSize(), 8);
  EXPECT_FALSE(http["connection"]);
}

TEST(HttpParserTest, Get1) {
  const std::string http_request{
      "GET / HTTP/1.1\r\n"
      "Host: reqbin.com\r\n"
      "Connection: keep-alive\r\n"
      "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
      "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.88 "
      "Safari/537.36\r\n"
      "Upgrade-Insecure-Requests: 1\r\n"
      "Accept: "
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/"
      "apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9\r\n"
      "Accept-Language: en-US,en;q=0.9\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "\r\n"};

  HttpParser http{http_request};
  EXPECT_TRUE(http.isValid());
  EXPECT_EQ(HttpMethod::Get, http.getMethod());
  EXPECT_EQ(http.getUri(), "/");
  ASSERT_FALSE(http["content-lenght"]);
  EXPECT_EQ(http.getResourceSize(), 0);
  EXPECT_TRUE(http["connection"]);
}

TEST(HttpParserTest, Get2) {
  const std::string http_request{
      "GET /index.html HTTP/1.1\r\n"
      "\r\n"};

  HttpParser http{http_request};
  EXPECT_TRUE(http.isValid());
  EXPECT_EQ(HttpMethod::Get, http.getMethod());
  EXPECT_EQ(http.getUri(), "/index.html");
  EXPECT_EQ(http.getResourceSize(), 0);
}

TEST(HttpParserTest, Delete) {
  const std::string http_request{
      "DELETE /echo/delete/json HTTP/1.1\r\n"
      "Host: reqbin.com\r\n"
      "\r\n"};

  HttpParser http{http_request};
  EXPECT_TRUE(http.isValid());
  EXPECT_EQ(HttpMethod::Delete, http.getMethod());
  EXPECT_EQ(http.getUri(), "/echo/delete/json");
  EXPECT_EQ(http.getResourceSize(), 0);
}

TEST(HttpParserTest, BasicAuthentication) {
  const std::string http_request{
      "DELETE /echo/delete/json HTTP/1.1\r\n"
      "Authorization: Bearer UmljazpzQW5jaGVa\r\n"
      "\r\n"};

  HttpParser http{http_request};
  ASSERT_TRUE(http.isValid());
  ASSERT_EQ(http["authorization"], "Bearer UmljazpzQW5jaGVa");
  ASSERT_TRUE(http.getAuthInfo());
  EXPECT_EQ(http.getAuthInfo()->username, "Rick");
  EXPECT_EQ(http.getAuthInfo()->password, "sAncheZ");
  EXPECT_EQ(http.getResourceSize(), 0);
}

TEST(HttpParserTest, BasicAuthenticationEmptyUserAndPass) {
  const std::string http_request{
      "GET /index.html HTTP/1.1\r\n"
      "Authorization: Bearer Og==\r\n"
      "\r\n"};

  HttpParser http{http_request};
  ASSERT_TRUE(http.isValid());
  ASSERT_EQ(http["authorization"], "Bearer Og==");
  ASSERT_TRUE(http.getAuthInfo());
  EXPECT_EQ(http.getAuthInfo()->username, "");
  EXPECT_EQ(http.getAuthInfo()->password, "");
  EXPECT_EQ(http.getResourceSize(), 0);
}

TEST(HttpParserTest, BasicAuthenticationNotBase64) {
  const std::string http_request{
      "GET /index.html HTTP/1.1\r\n"
      "Authorization: Basic Iam_not_base_64\r\n"
      "\r\n"};

  HttpParser http{http_request};
  ASSERT_TRUE(http.isValid());
  ASSERT_FALSE(http.getAuthInfo());
}

TEST(HttpParserTest, BasicAuthenticationMissing) {
  const std::string http_request{
      "GET /index.html HTTP/1.1\r\n"
      "\r\n"};

  HttpParser http{http_request};
  ASSERT_TRUE(http.isValid());
  ASSERT_FALSE(http.getAuthInfo());
}

TEST(HttpParserTest, MethodNotRecognised) {
  const std::string http_request{
      "POST /index.html HTTP/1.1\r\n"
      "\r\n"};

  const auto http = HttpParser(http_request);
  EXPECT_FALSE(http.isValid());
  EXPECT_EQ(HttpMethod::Unrecognized, http.getMethod());
  EXPECT_EQ(http.getResourceSize(), 0);
}
