#include "protocol/http/response/src/http_response.hpp"

#include "gtest/gtest.h"

using namespace protocol::http::response;

TEST(HttpResponseTest, Status) {
  HttpResponse http{HttpStatus::Created};
  ASSERT_EQ(std::string(http),
            "HTTP/1.1 201 Created\r\n"
            "Content-Length: 0\r\n"
            "\r\n");
}

TEST(HttpResponseTest, StatusAndReasonPhrase) {
  std::string reason{"I like trains"};
  HttpResponse http{reason, HttpStatus::NotFound};
  ASSERT_EQ(std::string(http),
            "HTTP/1.1 404 I like trains\r\n"
            "Content-Length: 0\r\n"
            "\r\n");
}

TEST(HttpResponseTest, StatusAndResource) {
  HttpResource resource{"http_ftp_server_project"};
  HttpResponse http{HttpStatus::Ok, resource};
  ASSERT_EQ(std::string(http),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/octet-stream\r\n"
            "Content-Length: 23\r\n"
            "\r\n"
            "http_ftp_server_project");
}

TEST(HttpResponseTest, StatusAndResponseHeadders) {
  HttpResponseHeaders headers{
      {"WWW-Authenticate", "Basic realm=\"User Visible Realm\""}};
  HttpResponse http{HttpStatus::Unauthorized, headers};
  ASSERT_EQ(std::string(http),
            "HTTP/1.1 401 Unauthorized\r\n"
            "WWW-Authenticate: Basic realm=\"User Visible Realm\"\r\n"
            "\r\n");
}

TEST(HttpResponseTest, PassAllParams) {
  HttpResponseHeaders headers{{"Content-Type", "text/html; charset=ISO-8859-1"},
                              {"Content-Encoding", "gzip"},
                              {"Server", "CAFE/1.0"},
                              {"Cache-control", "private, x-gzip-ok=\"\""},
                              {"Content-length", "1272"},
                              {"Date", "Thu, 13 May 2004 10:17:14 GMT"}};

  HttpResource resource{"x"};
  HttpResponse http{HttpStatus::Ok, "OK", headers, resource};
  ASSERT_EQ(std::string(http),
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html; charset=ISO-8859-1\r\n"
            "Content-Encoding: gzip\r\n"
            "Server: CAFE/1.0\r\n"
            "Cache-control: private, x-gzip-ok=\"\"\r\n"
            "Content-length: 1272\r\n"
            "Date: Thu, 13 May 2004 10:17:14 GMT\r\n"
            "\r\n"
            "x");
}