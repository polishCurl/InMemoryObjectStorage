#ifndef UTILS_SRC_UTILS_HPP
#define UTILS_SRC_UTILS_HPP

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

}  // namespace utils

#endif  // UTILS_SRC_UTILS_HPP
