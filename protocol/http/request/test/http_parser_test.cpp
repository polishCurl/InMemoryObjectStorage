#include "protocol/http/request/src/http_parser.hpp"

#include "gtest/gtest.h"

using namespace protocol::http::request;

TEST(HttpParserTest, InitialState) {
  const auto http = HttpParser("");
  EXPECT_FALSE(http.isValid());
  EXPECT_EQ(HttpMethod::Unrecognized, http.getMethod());
  EXPECT_TRUE(http.getUri().empty());
  EXPECT_FALSE(http.getResource());
  EXPECT_FALSE(http["content-lenght"]);
}

TEST(HttpParserTest, Put1) {
  std::string http_request{
      "PUT /xampp/tests/file/check.php HTTP/1.1\r\n"
      "Host: 127.0.0.1\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Lenght: 10\r\n"
      "Connection: close\r\n"
      "\r\n"
      "text1=sase"};

  HttpParser http{http_request};
  EXPECT_TRUE(http.isValid());
  EXPECT_EQ(HttpMethod::Put, http.getMethod());
  EXPECT_EQ(http.getUri(), "/xampp/tests/file/check.php");
  EXPECT_EQ(http.getResource(), "text1=sase");
  EXPECT_TRUE(http["content-lenght"]);
  EXPECT_EQ(http["host"], "127.0.0.1");
}

TEST(HttpParserTest, Put2) {
  const std::string http_request{
      "PUT /test HTTP/1.1\r\n"
      "Host: www.myServer.com\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Lenght: 8\r\n"
      "Accept: */*\r\n"
      "\r\n"
      "someData"};

  HttpParser http{http_request};
  EXPECT_TRUE(http.isValid());
  EXPECT_EQ(HttpMethod::Put, http.getMethod());
  EXPECT_EQ(http.getUri(), "/test");
  EXPECT_EQ(http.getResource(), "someData");
  EXPECT_EQ(http["content-lenght"], "8");
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
  EXPECT_FALSE(http.getResource());
  EXPECT_FALSE(http["content-lenght"]);
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
  EXPECT_FALSE(http.getResource());
}

TEST(HttpParserTest, Delete) {
  const std::string http_request{
      "DELETE /echo/delete/json HTTP/1.1\r\n"
      "Host: reqbin.com\r\n"
      "Authorization: Bearer mt0dgHmLJMVQhvjpNXDyA83vA_PxH23Y\r\n"
      "\r\n"};

  HttpParser http{http_request};
  EXPECT_TRUE(http.isValid());
  EXPECT_EQ(HttpMethod::Delete, http.getMethod());
  EXPECT_EQ(http.getUri(), "/echo/delete/json");
  EXPECT_FALSE(http.getResource());
}

TEST(HttpParserTest, MethodNotRecognised) {
  const std::string http_request{
      "POST /index.html HTTP/1.1\r\n"
      "\r\n"};

  const auto http = HttpParser(http_request);
  EXPECT_FALSE(http.isValid());
  EXPECT_EQ(HttpMethod::Unrecognized, http.getMethod());
}
