#include "http_parser.hpp"

namespace protocol {

namespace http {

namespace request {

static const std::unordered_map<std::string_view, HttpMethod> kMethodMap = {
    {"PUT", HttpMethod::Put},
    {"GET", HttpMethod::Get},
    {"DELETE", HttpMethod::Delete},
};

static const std::unordered_map<std::string_view, HttpVersion> kVersionMap = {
    {"HTTP/1.1", HttpVersion::HTTP_1_1},
};

static std::vector<std::string_view> split(const std::string& line,
                                           char delimeter) {
  std::vector<std::string_view> tokens;
  std::string_view line_view{line};
  auto end{line_view.find(delimeter)};
  decltype(end) start{0};

  while (end != std::string::npos) {
    tokens.push_back(line_view.substr(start, end - start));
    start = end + 1;
    end = line_view.find(delimeter, start);
  }

  tokens.push_back(line_view.substr(start, end - start));

  return tokens;
}

static void parseRequestLine(std::istringstream& ss, HttpRequest& http) {
  std::string request_line;
  if (std::getline(ss, request_line, '\n')) {
    request_line.pop_back();  // Remove CR character
    const auto tokens = split(request_line, ' ');
    http.method = kMethodMap.at(tokens[0]);
    http.uri = tokens[1];
    http.version = kVersionMap.at(tokens[2]);
  }
}

static void parseContentLength(std::istringstream& ss, HttpRequest& http) {
  std::string line;
  while (std::getline(ss, line, '\n') && (line != "\r")) {
    if (line.rfind("Content-Lenght", 0) == 0) {
      line.pop_back();  // Remove CR character
      const auto tokens = split(line, ' ');
      http.resource.size = std::atoi(tokens[1].data());
    }
  }
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
        parseContentLength(ss, http);
        http.resource.buffer = &buffer[ss.tellg()];
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
