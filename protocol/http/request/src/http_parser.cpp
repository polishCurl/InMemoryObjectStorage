#include "http_parser.hpp"

#include <sstream>
#include <unordered_map>
#include <vector>

#include "utils/src/utils.hpp"

namespace protocol {

namespace http {

namespace request {

static const std::unordered_map<std::string_view, HttpMethod> kMethodMap = {
    {"PUT", HttpMethod::Put},
    {"GET", HttpMethod::Get},
    {"DELETE", HttpMethod::Delete},
};

static const std::unordered_map<std::string_view, HttpVersion> kVersionMap = {
    {"HTTP/1.1", HttpVersion::Http_1_1},
};

static void parseRequestLine(std::istringstream& ss, HttpRequest& http) {
  std::string request_line;
  if (std::getline(ss, request_line, '\n')) {
    request_line.pop_back();  // Remove CR character
    const auto tokens = utils::split(request_line, ' ');
    http.method = kMethodMap.at(tokens[0]);
    http.uri = tokens[1];
    http.version = kVersionMap.at(tokens[2]);
  }
}

static std::size_t parseContentLength(std::istringstream& ss,
                                      HttpRequest& http) {
  std::string line;
  std::size_t content_length{0};
  while (std::getline(ss, line, '\n') && (line != "\r")) {
    if (line.rfind("Content-Lenght", 0) == 0) {
      const auto tokens = utils::split(line, ' ');
      content_length = std::atoi(tokens[1].data());
    }
  }

  return content_length;
}

HttpRequest parseHttp(const std::string& buffer) noexcept {
  HttpRequest http{false,
                   HttpVersion::Unrecognized,
                   HttpMethod::Unrecognized,
                   "",
                   {nullptr, 0}};

  if (!buffer.empty()) {
    try {
      std::istringstream ss{buffer};
      parseRequestLine(ss, http);

      if (http.method == HttpMethod::Put) {
        const auto content_length = parseContentLength(ss, http);
        http.resource = {&buffer[ss.tellg()], content_length};
      }
      http.valid = true;
    } catch (std::exception& e) {
      http.valid = false;
    }
  }

  return http;
}

}  // namespace request
}  // namespace http
}  // namespace protocol
