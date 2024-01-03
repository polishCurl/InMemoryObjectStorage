#ifndef PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP
#define PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP

#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace protocol {

namespace http {

namespace request {

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
 * \brief Parse HTTP request.
 *
 * Extract relevant information from HTTP request.
 *
 * \param buffer Input buffer containing HTTP request.
 *
 * \return Parsed HTTP request.
 */
HttpRequest parseHttp(const std::string& buffer) noexcept;

}  // namespace request
}  // namespace http
}  // namespace protocol

#endif  // PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP
