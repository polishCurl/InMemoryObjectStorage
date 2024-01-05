#include "utils.hpp"

namespace utils {

std::vector<std::string_view> split(std::string_view text,
                                    std::string_view delimeter) noexcept {
  if (text.empty()) {
    return {};
  }

  std::vector<std::string_view> tokens;
  auto end{text.find(delimeter)};
  decltype(end) start{0};

  while (end != std::string::npos) {
    tokens.push_back(text.substr(start, end - start));
    start = end + delimeter.size();
    end = text.find(delimeter, start);
  }

  tokens.push_back(text.substr(start, end - start));

  return tokens;
}

}  // namespace utils