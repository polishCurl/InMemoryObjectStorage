#include "http_parser.hpp"

#include "utils/src/utils.hpp"

namespace protocol {

namespace http {

namespace request {

const std::unordered_map<std::string_view, HttpMethod> HttpParser::kMethodMap =
    {
        {"PUT", HttpMethod::Put},
        {"GET", HttpMethod::Get},
        {"DELETE", HttpMethod::Delete},
};

HttpParser::HttpParser(const std::string& buffer) noexcept
    : valid_{false}, method_{HttpMethod::Unrecognized} {
  if (!buffer.empty()) {
    try {
      std::vector<std::string_view> lines{utils::split(buffer, "\r\n")};
      parseRequestLine(lines);
      parseHeaderFields(lines);

      if (!lines.back().empty()) {
        resource_ = lines.back();
      }

      valid_ = true;
    } catch (std::exception& e) {
    }
  }
}

std::optional<std::string_view> HttpParser::operator[](
    const std::string& key) const noexcept {
  if (header_fields_.find(key) == header_fields_.end()) {
    return {};
  }

  return header_fields_.at(key);
}

void HttpParser::parseRequestLine(const std::vector<std::string_view>& lines) {
  const auto tokens = utils::split(lines[0], " ");
  method_ = kMethodMap.at(tokens[0]);
  uri_ = tokens[1];
}

void HttpParser::parseHeaderFields(const std::vector<std::string_view>& lines) {
  for (auto line = lines.cbegin() + 1; line != lines.cend() - 2; line++) {
    auto tokens = utils::split(*line, ":");

    // Remove leading whitespaces from value.
    tokens[1].remove_prefix(tokens[1].find_first_not_of(' '));

    // Convert name to lowercase.
    std::string name{tokens[0]};
    utils::toLowerCase(name);
    header_fields_[name] = tokens[1];
  }
}

}  // namespace request
}  // namespace http
}  // namespace protocol
