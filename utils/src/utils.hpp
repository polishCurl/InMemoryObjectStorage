#ifndef UTILS_SRC_UTILS_HPP
#define UTILS_SRC_UTILS_HPP

#include <optional>
#include <string>
#include <vector>

namespace utils {

/**
 * \brief Split string into tokens based on a delimiter.
 *
 * \param text Input string.
 * \param delimeter Delimiter based on which to split the string.
 *
 * \return List of tokens.
 */
std::vector<std::string_view> split(std::string_view text,
                                    std::string_view delimeter) noexcept;

/**
 * \brief Convert string to lower case.
 *
 * \param text Input string.
 */
void toLowerCase(std::string& text) noexcept;

/**
 * \brief Convert string to upper case.
 *
 * \param text Input string.
 */
void toUpperCase(std::string& text) noexcept;

/**
 * \brief Decode a base64-encoded string
 *
 * \param input Input string encoded in Base 64.
 *
 * \return Decoded string, if decoding was successful.
 */
std::optional<std::string> decode_base64(const std::string& input);

}  // namespace utils

#endif  // UTILS_SRC_UTILS_HPP
