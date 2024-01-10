#ifndef PROTOCOL_FTP_REQUEST_SRC_FTP_PARSER_HPP
#define PROTOCOL_FTP_REQUEST_SRC_FTP_PARSER_HPP

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "protocol/ivalidate.hpp"

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
  User,
  Pass,
  Unrecognized,
};

/**
 * \brief FTP request parser.
 *
 * Extracts relevant information from FTP request.
 */
class FtpParser : public IValidate {
 public:
  /**
   * \brief Create FTP parser.
   *
   * \param buffer Raw input buffer containing FTP request.
   */
  FtpParser(const std::string& buffer) noexcept;

  FtpParser(const FtpParser& other) = delete;
  FtpParser(FtpParser&& other) = delete;
  FtpParser& operator=(const FtpParser& other) = delete;
  FtpParser& operator=(FtpParser&&) = delete;

  /**
   * \brief Return whether the FTP request was successfully parsed.
   *
   * \return True if FTP request was successfully parsed, false otherwise.
   */
  inline bool isValid() const noexcept override { return valid_; }

  /**
   * \brief Return FTP command.
   *
   * \return FTP command.
   */
  inline FtpCommand getCommand() const noexcept { return command_; }

  /**
   * \brief Return the list of tokens (FTP command name and all arguments).
   *
   * \return FTP command tokens.
   */
  inline const std::vector<std::string_view>& getTokens() const noexcept {
    return tokens_;
  }

 private:
  /// Mapping from string representation of FTP command to the decoded value.
  static const std::unordered_map<std::string, FtpCommand> kCommandMap;

  bool valid_;                            ///< Is FTP request valid?
  FtpCommand command_;                    ///< Decoded FTP command
  std::vector<std::string_view> tokens_;  ///< Parsed FTP command tokens
};

}  // namespace request
}  // namespace ftp
}  // namespace protocol

#endif  // PROTOCOL_FTP_REQUEST_SRC_FTP_PARSER_HPP
