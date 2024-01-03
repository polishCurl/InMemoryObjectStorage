#include "src/protocol_detector.hpp"

#include "gtest/gtest.h"

using namespace protocol;

TEST(ProtocolDetectorTest, HttpPut) {
  char http_header[] =
      "PUT /xampp/tests/file/check.php HTTP/1.1\r\n"
      "Host: 127.0.0.1\r\n"
      "Content-Type: application/x-www-form-urlencoded\r\n"
      "Content-Lenght: 10\r\n"
      "Connection: close\r\n"
      "\r\n"
      "text1=sase";

  EXPECT_EQ(AppLayerProtocol::Http, ProtocolDetector{http_header}.detect());
}

TEST(ProtocolDetectorTest, HttpGet) {
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

  EXPECT_EQ(AppLayerProtocol::Http, ProtocolDetector{http_header}.detect());
}

TEST(ProtocolDetectorTest, HttpGet2) {
  char http_header[] =
      "GET /index.html HTTP/2.0\r\n"
      "\r\n";

  EXPECT_EQ(AppLayerProtocol::Http, ProtocolDetector{http_header}.detect());
}

TEST(ProtocolDetectorTest, FtpUser) {
  char ftp_header[] = "USER csanders\r\n";
  EXPECT_EQ(AppLayerProtocol::Ftp, ProtocolDetector{ftp_header}.detect());
}

TEST(ProtocolDetectorTest, FtpPwd) {
  char ftp_header[] = "PWD\r\n";
  EXPECT_EQ(AppLayerProtocol::Ftp, ProtocolDetector{ftp_header}.detect());
}

TEST(ProtocolDetectorTest, FtpRetr) {
  char ftp_header[] = "RETR Music.mp3\r\n";
  EXPECT_EQ(AppLayerProtocol::Ftp, ProtocolDetector{ftp_header}.detect());
}
