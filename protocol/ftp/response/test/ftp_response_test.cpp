#include "src/ftp_response.hpp"

#include "gtest/gtest.h"

using namespace protocol::ftp;

TEST(FtpResponseTest, NoMessage) {
  FtpResponse ftp_resp{FtpReplyCode::COMMAND_OK};
  ASSERT_EQ(std::string(ftp_resp), "200 \r\n");
}

TEST(FtpResponseTest, CustomMessage) {
  FtpResponse ftp_resp{
      FtpReplyCode::FILE_STATUS_OK_OPENING_DATA_CONNECTION,
      "Response arg: Data connection accepted from 192.168.0.114:1140; "
      "transfer starting for Music.mp3 (4980924 bytes)."};
  ASSERT_EQ(
      std::string(ftp_resp),
      "150 Response arg: Data connection accepted from 192.168.0.114:1140; "
      "transfer starting for Music.mp3 (4980924 bytes).\r\n");
}