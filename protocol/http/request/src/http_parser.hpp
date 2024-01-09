#ifndef PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP
#define PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "protocol/imember_access.hpp"
#include "protocol/ivalidate.hpp"

namespace protocol {

namespace http {

namespace request {

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
 * \brief HTTP basic authentication info.
 */
struct HttpAuthInfo {
  std::string username;  ///< Username
  std::string password;  ///< Password
};

/**
 * \brief HTTP request parser.
 *
 * Extracts relevant information from HTTP request.
 */
class HttpParser : public IMemberAccess, public IValidate {
 public:
  /**
   * \brief Create HTTP parser.
   *
   * \param buffer Raw input buffer containing HTTP request.
   */
  HttpParser(const std::string& buffer) noexcept;

  HttpParser(const HttpParser& other) = delete;
  HttpParser(HttpParser&& other) = delete;
  HttpParser& operator=(const HttpParser& other) = delete;
  HttpParser& operator=(HttpParser&&) = delete;

  /**
   * \brief Return whether the HTTP request was successfully parsed.
   *
   * \return True if HTTP request was successfully parsed, false otherwise.
   */
  inline bool isValid() const noexcept override { return valid_; }

  /**
   * \brief Return HTTP method.
   *
   * \return HTTP method.
   */
  inline HttpMethod getMethod() const noexcept { return method_; }

  /**
   * \brief Return Uniform Resource Identifier (URI)
   *
   * \return URI.
   */
  inline std::string_view getUri() const noexcept { return uri_; }

  /**
   * \brief Return HTTP resource size.
   *
   * \return HTTP resource size (0 if no message-body).
   */
  inline std::size_t getResourceSize() const noexcept { return resource_size_; }

  /**
   * \brief Access HTTP request header field value by its name.
   *
   * \note Use all-lowercase header name, e.g. 'content-type'.
   *
   * \return HTTP request header field value, if present.
   */
  std::optional<std::string_view> operator[](
      const std::string&) const noexcept override;

  /**
   * \brief Return HTTP basic authentication info.
   *
   * \return HTTP Authorization header value, if present.
   */
  std::optional<HttpAuthInfo> getAuthInfo() const noexcept;

 private:
  /// Mapping ftom HTTP request header field name to value.
  using HttpHeaderFields = std::unordered_map<std::string, std::string_view>;

  /// HTTP request header name for getting the resource size (content length).
  static constexpr std::string_view kContentLengthKey{"content-length"};

  /// HTTP request header name for getting the HTTP basic authentication info.
  static constexpr std::string_view kAuthenticationKey{"authorization"};

  /// Mapping from string representation of HTTP method to the decoded value.
  static const std::unordered_map<std::string_view, HttpMethod> kMethodMap;

  void parseRequestLine(const std::vector<std::string_view>& lines);
  void parseHeaderFields(const std::vector<std::string_view>& lines);

  bool valid_;                      ///< Is HTTP request valid?
  HttpMethod method_;               ///< HTTP request method
  std::string_view uri_;            ///< HTTP URI
  std::size_t resource_size_;       ///< HTTP resource size.
  HttpHeaderFields header_fields_;  ///< HTTP header fields.
};

}  // namespace request
}  // namespace http
}  // namespace protocol

#endif  // PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP
