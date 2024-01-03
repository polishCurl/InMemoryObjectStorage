#include "protocol/ftp/request/src/ftp_parser.hpp"

#include "gtest/gtest.h"

using namespace protocol::ftp::request;

TEST(FtpParserTest, InitialState) {
  const auto ftp = parseFtp("");
  EXPECT_FALSE(ftp.valid);
  EXPECT_EQ(FtpCommand::Unrecognized, ftp.command);
  EXPECT_TRUE(ftp.argument.empty());
}

TEST(FtpParserTest, Invalid) {
  char http_header[] =
      "GET /index.html HTTP/2.0\r\n"
      "\r\n";
  const auto ftp = parseFtp(http_header);
  EXPECT_FALSE(ftp.valid);
  EXPECT_EQ(FtpCommand::Unrecognized, ftp.command);
  EXPECT_TRUE(ftp.argument.empty());
}

TEST(FtpParserTest, UnrecognizedCommand) {
  const auto ftp = parseFtp("SITE CHMOD 777 resume.doc\r\n");
  EXPECT_FALSE(ftp.valid);
  EXPECT_EQ(FtpCommand::Unrecognized, ftp.command);
  EXPECT_TRUE(ftp.argument.empty());
}

TEST(FtpParserTest, List) {
  const auto ftp = parseFtp("LIST\r\n");
  EXPECT_TRUE(ftp.valid);
  EXPECT_EQ(FtpCommand::List, ftp.command);
  EXPECT_TRUE(ftp.argument.empty());
}

TEST(FtpParserTest, Retr) {
  char ftp_header[] = "Retr /resume_cv.doc\r\n";
  const auto ftp = parseFtp(ftp_header);
  EXPECT_TRUE(ftp.valid);
  EXPECT_EQ(FtpCommand::Retr, ftp.command);
  EXPECT_EQ(ftp.argument, "/resume_cv.doc");
}

TEST(FtpParserTest, Stor) {
  char ftp_header[] = "stor /some/file/path/file.txt\r\n";
  const auto ftp = parseFtp(ftp_header);
  EXPECT_TRUE(ftp.valid);
  EXPECT_EQ(FtpCommand::Stor, ftp.command);
  EXPECT_EQ(ftp.argument, "/some/file/path/file.txt");
}

TEST(FtpParserTest, Dele) {
  char ftp_header[] = "Dele resume.doc\r\n";
  const auto ftp = parseFtp(ftp_header);
  EXPECT_TRUE(ftp.valid);
  EXPECT_EQ(FtpCommand::Dele, ftp.command);
  EXPECT_EQ(ftp.argument, "resume.doc");
}