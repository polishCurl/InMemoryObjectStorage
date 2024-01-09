#include "utils.hpp"

#include <algorithm>

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

void toLowerCase(std::string& text) noexcept {
  std::transform(text.begin(), text.end(), text.begin(), ::tolower);
}

void toUpperCase(std::string& text) noexcept {
  std::transform(text.begin(), text.end(), text.begin(), ::toupper);
}

std::optional<std::string> decode_base64(const std::string& input) {
  static constexpr unsigned char kDecodingTable[] = {
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52, 53, 54, 55, 56, 57,
      58, 59, 60, 61, 64, 64, 64, 64, 64, 64, 64, 0,  1,  2,  3,  4,  5,  6,
      7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,
      25, 64, 64, 64, 64, 64, 64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36,
      37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
      64, 64, 64, 64};

  auto input_length = input.size();
  if (input_length % 4 != 0) {
    return {};
  }

  auto output_length = input_length / 4 * 3;
  if (input[input_length - 1] == '=') output_length--;
  if (input[input_length - 2] == '=') output_length--;

  std::string output;
  output.resize(output_length);

  for (std::size_t i = 0, j = 0; i < input_length;) {
    std::uint32_t a = input[i] == '='
                          ? 0 & i++
                          : kDecodingTable[static_cast<int>(input[i++])];
    std::uint32_t b = input[i] == '='
                          ? 0 & i++
                          : kDecodingTable[static_cast<int>(input[i++])];
    std::uint32_t c = input[i] == '='
                          ? 0 & i++
                          : kDecodingTable[static_cast<int>(input[i++])];
    std::uint32_t d = input[i] == '='
                          ? 0 & i++
                          : kDecodingTable[static_cast<int>(input[i++])];

    std::uint32_t triple =
        (a << 3 * 6) + (b << 2 * 6) + (c << 1 * 6) + (d << 0 * 6);

    if (j < output_length) output[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < output_length) output[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < output_length) output[j++] = (triple >> 0 * 8) & 0xFF;
  }

  return output;
}

}  // namespace utils