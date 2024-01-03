#ifndef UTILS_SRC_UTILS_HPP
#define UTILS_SRC_UTILS_HPP

#include <string>
#include <vector>

namespace utils {

/**
 * \brief Split string into tokens based on a delimiter.
 *
 * Extract relevant information from FTP request.
 *
 * \param text Input string.
 * \param delimeter Delimiter based on which to split the string.
 *
 * \return List of tokens.
 */
std::vector<std::string_view> split(std::string_view text, char delimeter);

}  // namespace utils

#endif  // UTILS_SRC_UTILS_HPP
