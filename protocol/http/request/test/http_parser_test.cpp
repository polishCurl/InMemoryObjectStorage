#include "src/http_parser.hpp"

#include "gtest/gtest.h"

using namespace protocol::http;

TEST(HttpParserTest, InitialState) {
  HttpParser parser{""};
  const auto& http = parser.parse();
  EXPECT_FALSE(http.valid);
  EXPECT_EQ(HttpVersion::Unrecognized, http.version);
  EXPECT_EQ(HttpMethod::Unrecognized, http.method);
  EXPECT_EQ("", http.uri);
  EXPECT_EQ(nullptr, http.resource.buffer);
  EXPECT_EQ(0, http.resource.size);
}

TEST(HttpParserTest, Put1) {
  char http_header[] =
      "PUT /xampp/tests/file/check.php HTTP/1.1\r\n"
      "Host: 127.0.0.1\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Lenght: 10\r\n"
      "Connection: close\r\n"
      "\r\n"
      "text1=sase";

  HttpParser parser{http_header};
  const auto& http = parser.parse();
  EXPECT_TRUE(http.valid);
  EXPECT_EQ(HttpVersion::HTTP_1_1, http.version);
  EXPECT_EQ(HttpMethod::Put, http.method);
  EXPECT_EQ("/xampp/tests/file/check.php", http.uri);
  ASSERT_EQ(10, http.resource.size);
  EXPECT_EQ(
      0, std::memcmp("text1=sase", http.resource.buffer, http.resource.size));
}

TEST(HttpParserTest, Put2) {
  char http_header[] =
      "PUT /test HTTP/1.1\r\n"
      "Host: www.myServer.com\r\n"
      "Content-Type: text/plain\r\n"
      "Content-Lenght: 8\r\n"
      "Accept: */*\r\n"
      "\r\n"
      "someData";

  HttpParser parser{http_header};
  const auto& http = parser.parse();
  EXPECT_TRUE(http.valid);
  EXPECT_EQ(HttpVersion::HTTP_1_1, http.version);
  EXPECT_EQ(HttpMethod::Put, http.method);
  EXPECT_EQ("/test", http.uri);
  ASSERT_EQ(8, http.resource.size);
  EXPECT_EQ(0,
            std::memcmp("someData", http.resource.buffer, http.resource.size));
}

TEST(HttpParserTest, Get1) {
  char http_header[] =
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
      "\r\n";

  HttpParser parser{http_header};
  const auto& http = parser.parse();
  EXPECT_TRUE(http.valid);
  EXPECT_EQ(HttpVersion::HTTP_1_1, http.version);
  EXPECT_EQ(HttpMethod::Get, http.method);
  EXPECT_EQ("/", http.uri);
  EXPECT_EQ(nullptr, http.resource.buffer);
  EXPECT_EQ(0, http.resource.size);
}

TEST(HttpParserTest, Get2) {
  char http_header[] =
      "GET /index.html HTTP/1.1\r\n"
      "\r\n";

  HttpParser parser{http_header};
  const auto& http = parser.parse();
  EXPECT_TRUE(http.valid);
  EXPECT_EQ(HttpVersion::HTTP_1_1, http.version);
  EXPECT_EQ(HttpMethod::Get, http.method);
  EXPECT_EQ("/index.html", http.uri);
  EXPECT_EQ(nullptr, http.resource.buffer);
  EXPECT_EQ(0, http.resource.size);
}

TEST(HttpParserTest, Delete) {
  char http_header[] =
      "DELETE /echo/delete/json HTTP/1.1\r\n"
      "Host: reqbin.com\r\n"
      "Authorization: Bearer mt0dgHmLJMVQhvjpNXDyA83vA_PxH23Y\r\n"
      "\r\n";

  HttpParser parser{http_header};
  const auto& http = parser.parse();
  EXPECT_TRUE(http.valid);
  EXPECT_EQ(HttpVersion::HTTP_1_1, http.version);
  EXPECT_EQ(HttpMethod::Delete, http.method);
  EXPECT_EQ("/echo/delete/json", http.uri);
  EXPECT_EQ(nullptr, http.resource.buffer);
  EXPECT_EQ(0, http.resource.size);
}

TEST(HttpParserTest, VersionNotRecognised) {
  char http_header[] =
      "GET /index.html HTTP/2.0\r\n"
      "\r\n";

  HttpParser parser{http_header};
  const auto& http = parser.parse();
  EXPECT_FALSE(http.valid);
  EXPECT_EQ(HttpVersion::Unrecognized, http.version);
}

TEST(HttpParserTest, MethodNotRecognised) {
  char http_header[] =
      "POST /index.html HTTP/1.1\r\n"
      "\r\n";

  HttpParser parser{http_header};
  const auto& http = parser.parse();
  EXPECT_FALSE(http.valid);
  EXPECT_EQ(HttpMethod::Unrecognized, http.method);
}