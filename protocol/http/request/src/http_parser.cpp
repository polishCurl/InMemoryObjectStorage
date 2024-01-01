#include "http_parser.hpp"

#include <iostream>
#include <vector>

namespace protocol {

namespace http {

HttpParser::HttpParser(const char* buffer, std::size_t size)
    : http_{false,
            HttpVersion::Unrecognized,
            HttpMethod::Unrecognized,
            "",
            {nullptr, 0}} {
  if (!buffer) {
    return;
  }

  try {
    std::istringstream ss{buffer};
    parseRequestLine(ss);

    if (http_.method == HttpMethod::Put) {
      parseContentLength(ss);
      http_.resource.buffer = &buffer[ss.tellg()];
    }
    http_.valid = true;
  } catch (std::exception& e) {
    std::cerr << "Failed to parse: " << buffer << '\n' << e.what() << '\n';
  }
}

void HttpParser::parseRequestLine(std::istringstream& ss) {
  std::string request_line;
  if (std::getline(ss, request_line, '\n')) {
    request_line.pop_back();  // Remove CR character
    const auto tokens = split(request_line, ' ');
    http_.method = kMethodMap.at(tokens[0]);
    http_.uri = tokens[1];
    http_.version = kVersionMap.at(tokens[2]);
  }
}

void HttpParser::parseContentLength(std::istringstream& ss) {
  std::string line;
  while (std::getline(ss, line, '\n') && (line != "\r")) {
    if (line.rfind("Content-Lenght", 0) == 0) {
      line.pop_back();  // Remove CR character
      const auto tokens = split(line, ' ');
      http_.resource.size = std::atoi(tokens[1].data());
    }
  }
}

std::vector<std::string_view> HttpParser::split(const std::string& line,
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

const std::unordered_map<std::string_view, HttpMethod> HttpParser::kMethodMap =
    {
        {"PUT", HttpMethod::Put},
        {"GET", HttpMethod::Get},
        {"DELETE", HttpMethod::Delete},
};

const std::unordered_map<std::string_view, HttpVersion>
    HttpParser::kVersionMap = {
        {"HTTP/1.1", HttpVersion::HTTP_1_1},
};

}  // namespace http
}  // namespace protocol
