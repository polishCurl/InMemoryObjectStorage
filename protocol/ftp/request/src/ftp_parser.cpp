#include "ftp_parser.hpp"

#include <algorithm>
#include <iostream>
#include <unordered_map>

#include "utils/src/utils.hpp"

namespace protocol {

namespace ftp {

namespace request {

static const std::unordered_map<std::string, FtpCommand> kCommandMap = {
    {"LIST", FtpCommand::List},
    {"RETR", FtpCommand::Retr},
    {"STOR", FtpCommand::Stor},
    {"DELE", FtpCommand::Dele},
};

FtpRequest parseFtp(const std::string& buffer) noexcept {
  FtpRequest ftp{false, FtpCommand::Unrecognized, ""};

  if (!buffer.empty()) {
    try {
      std::string_view str_view{buffer};
      str_view.remove_suffix(2);
      const auto tokens = utils::split(str_view, ' ');

      for (auto el : tokens) {
        std::cout << el << ' ';
      }

      std::cout << '\t';

      std::string command{tokens[0]};
      std::transform(command.begin(), command.end(), command.begin(),
                     ::toupper);

      std::cout << command << '\n';
      ftp.command = kCommandMap.at(command);

      if (tokens.size() > 1) {
        ftp.argument = tokens[1];
      }

      ftp.valid = true;
    } catch (std::exception& e) {
      ftp.valid = false;
    }
  }

  return ftp;
}

}  // namespace request
}  // namespace ftp
}  // namespace protocol
