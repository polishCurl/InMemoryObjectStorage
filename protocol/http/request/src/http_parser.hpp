#ifndef PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP
#define PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace protocol {

namespace http {

/**
 * \brief HTTP versions supported.
 */
enum class HttpVersion {
  HTTP_1_1,
  Unrecognized,
};

/**
 * \brief HTTP methods supported.
 */
enum class HttpMethod {
  Get,
  Put,
  Delete,
  Unrecognized,
};

/**
 * \brief HTTP resource representation.
 */
struct HttpResource {
  const char* buffer;  //*!< HttpResource buffer
  std::size_t size;    //*!< HttpResource buffer size
};

/**
 * \brief Parsed HTTP request.
 */
struct HttpRequest {
  bool valid;             //*!< Is HTTP request is valid? (successfully parsed)
  HttpVersion version;    //*!< HTTP version
  HttpMethod method;      //*!< HTTP method
  std::string uri;        //*!< URI
  HttpResource resource;  //*!< HTTP request body (the resource), if present
};

/**
 * \brief HTTP request parser.
 */
class HttpParser {
 public:
  /**
   * \brief Create HTTP parser from raw HTTP buffer
   *
   * \param buffer Input buffer containing HTTP request.
   * \param size Size of buffer.
   */
  HttpParser(const char* buffer, std::size_t size);

  // For now, I don't see the need for providing copy and move semantics
  HttpParser(const HttpParser& other) = delete;
  HttpParser(HttpParser&& other) = delete;
  HttpParser& operator=(const HttpParser& other) = delete;
  HttpParser& operator=(HttpParser&&) = delete;

  /**
   * \brief Parse HTTP request.
   *
   * Extract relevant information from HTTP request.
   *
   * \return Parsed HTTP request.
   */
  inline const HttpRequest& parse() const noexcept { return http_; }

 protected:
  /**
   * \brief Parse HTTP request line.
   *
   * Extract relevant data from HTTP request line (1st line).
   *
   * \param ss Input string stream which contains HTTP request.
   */
  void parseRequestLine(std::istringstream& ss);

  /**
   * \brief Parse HTTP content length.
   *
   * \param ss Input string stream which contains HTTP request.
   */
  void parseContentLength(std::istringstream& ss);

  /**
   * \brief Split line into tokens based on a delimiter
   *
   * Extract relevant data from HTTP request line (1st line).
   *
   * \param line Line to split.
   * \param delimeter Character on based on which to split the string.
   *
   * \return List of tokens.
   */
  static std::vector<std::string_view> split(const std::string& line,
                                             char delimeter);

  HttpRequest http_;  //*!< Information extracted from HTTP request.

  /// Mapping from HTTP method name to the corresponding type.
  static const std::unordered_map<std::string_view, HttpMethod> kMethodMap;

  /// Mapping from HTTP version name to the corresponding type.
  static const std::unordered_map<std::string_view, HttpVersion> kVersionMap;
};

}  // namespace http
}  // namespace protocol

#endif  // PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP
