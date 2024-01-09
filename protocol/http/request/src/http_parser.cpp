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
    : valid_{false}, method_{HttpMethod::Unrecognized}, resource_size_{0} {
  if (!buffer.empty()) {
    try {
      std::vector<std::string_view> lines{utils::split(buffer, "\r\n")};
      parseRequestLine(lines);
      parseHeaderFields(lines);

      if (std::string key{kContentLengthKey};
          header_fields_.find(key) != header_fields_.end()) {
        resource_size_ = std::atoi(header_fields_.at(key).data());
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

std::optional<HttpAuthInfo> HttpParser::getAuthInfo() const noexcept {
  std::string key{kAuthenticationKey};
  const auto value = (*this)[key];
  if (!value) {
    return {};
  }

  const auto auth_data = std::string{utils::split(*value, " ")[1]};
  const auto decoded_auth_data = utils::decode_base64(auth_data);
  if (!decoded_auth_data) {
    return {};
  }

  const auto user_and_pass = utils::split(*decoded_auth_data, ":");
  return HttpAuthInfo{std::string{user_and_pass[0]},
                      std::string{user_and_pass[1]}};
}

}  // namespace request
}  // namespace http
}  // namespace protocol
