#include "ftp_parser.hpp"

#include "utils/src/utils.hpp"

namespace protocol {

namespace ftp {

namespace request {

FtpParser::FtpParser(const std::string& buffer) noexcept
    : valid_{false}, command_{FtpCommand::Unrecognized} {
  if (!buffer.empty()) {
    try {
      std::string_view buffer_view{buffer};
      buffer_view.remove_suffix(2);  // Remove "\r\n"
      tokens_ = utils::split(buffer_view, " ");
      std::string command{tokens_[0]};
      utils::toUpperCase(command);
      command_ = kCommandMap.at(command);
      valid_ = true;
    } catch (std::exception& e) {
    }
  }
}

const std::unordered_map<std::string, FtpCommand> FtpParser::kCommandMap = {
    {"LIST", FtpCommand::List}, {"RETR", FtpCommand::Retr},
    {"STOR", FtpCommand::Stor}, {"DELE", FtpCommand::Dele},
    {"PASS", FtpCommand::Pass}, {"USER", FtpCommand::User},
};

}  // namespace request
}  // namespace ftp
}  // namespace protocol
