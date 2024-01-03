#ifndef PROTOCOL_FTP_REQUEST_SRC_FTP_PARSER_HPP
#define PROTOCOL_FTP_REQUEST_SRC_FTP_PARSER_HPP

#include <string>

namespace protocol {

namespace ftp {

namespace request {

/**
 * \brief FTP request commands supported.
 */
enum class FtpCommand {
  List,
  Retr,
  Stor,
  Dele,
  Unrecognized,
};

/**
 * \brief Parsed FTP request.
 *
 * \note There are few FTP commands which have more than one argument but I
 * chose not to support this.
 */
struct FtpRequest {
  bool valid;                 //*!< Is FTP request valid? (successfully parsed)
  FtpCommand command;         //*!< FTP command
  std::string_view argument;  //*!< FTP command argument
};

/**
 * \brief Parse FTP request.
 *
 * Extract relevant information from FTP request.
 *
 * \param buffer Input buffer containing FTP request.
 *
 * \return Parsed FTP request.
 */
FtpRequest parseFtp(const std::string& buffer) noexcept;

}  // namespace request
}  // namespace ftp
}  // namespace protocol

#endif  // PROTOCOL_FTP_REQUEST_SRC_FTP_PARSER_HPP
