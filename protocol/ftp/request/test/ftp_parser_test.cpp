#include "protocol/ftp/request/src/ftp_parser.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace protocol::ftp::request;

TEST(FtpParserTest, InitialState) {
  FtpParser ftp{""};
  ASSERT_FALSE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::Unrecognized);
}

TEST(FtpParserTest, Invalid) {
  std::string http_header{
      "GET /index.html HTTP/2.0\r\n"
      "\r\n"};
  FtpParser ftp{http_header};
  ASSERT_FALSE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::Unrecognized);
}

TEST(FtpParserTest, UnrecognizedCommand) {
  std::string ftp_request{"SITE CHMOD 777 resume.doc\r\n"};
  FtpParser ftp{ftp_request};
  ASSERT_FALSE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::Unrecognized);
}

TEST(FtpParserTest, List) {
  std::string ftp_request{"LIST\r\n"};
  FtpParser ftp{ftp_request};
  ASSERT_TRUE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::List);
  EXPECT_THAT(ftp.getTokens(), ::testing::ElementsAreArray({"LIST"}));
}

TEST(FtpParserTest, Retr) {
  const std::string ftp_request{"Retr /resume_cv.doc\r\n"};
  FtpParser ftp{ftp_request};
  ASSERT_TRUE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::Retr);
  EXPECT_THAT(ftp.getTokens(),
              ::testing::ElementsAreArray({"Retr", "/resume_cv.doc"}));
}

TEST(FtpParserTest, Stor) {
  std::string ftp_request{
      "stor iClikeCtrainAAAAAAAAA_^-27163_very_loOO-ng\r\n"};
  FtpParser ftp{ftp_request};
  ASSERT_TRUE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::Stor);
  EXPECT_THAT(ftp.getTokens(),
              ::testing::ElementsAreArray(
                  {"stor", "iClikeCtrainAAAAAAAAA_^-27163_very_loOO-ng"}));
}

TEST(FtpParserTest, Dele) {
  std::string ftp_request{"Dele resume.doc\r\n"};
  FtpParser ftp{ftp_request};
  ASSERT_TRUE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::Dele);
  EXPECT_THAT(ftp.getTokens(),
              ::testing::ElementsAreArray({"Dele", "resume.doc"}));
}

TEST(FtpParserTest, User) {
  std::string ftp_request{"uSER anonymous\r\n"};
  FtpParser ftp{ftp_request};
  ASSERT_TRUE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::User);
  EXPECT_THAT(ftp.getTokens(),
              ::testing::ElementsAreArray({"uSER", "anonymous"}));
}

TEST(FtpParserTest, Pass) {
  std::string ftp_request{"PASS admin4321\r\n"};
  FtpParser ftp{ftp_request};
  ASSERT_TRUE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::Pass);
  EXPECT_THAT(ftp.getTokens(),
              ::testing::ElementsAreArray({"PASS", "admin4321"}));
}

TEST(FtpParserTest, Pasv) {
  std::string ftp_request{"PASV\r\n"};
  FtpParser ftp{ftp_request};
  ASSERT_TRUE(ftp.isValid());
  EXPECT_EQ(ftp.getCommand(), FtpCommand::Pasv);
  EXPECT_THAT(ftp.getTokens(), ::testing::ElementsAreArray({"PASV"}));
}