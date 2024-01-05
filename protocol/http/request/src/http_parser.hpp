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
 * \brief HTTP resource representation.
 */
using HttpResource = std::string_view;

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
   * \brief Return HTTP resource (message body).
   *
   * \return HTTP resource if present.
   */
  inline std::optional<HttpResource> getResource() const noexcept {
    return resource_;
  }

  /**
   * \brief Access HTTP request header field value by its name.
   *
   * \note Use all-lowercase header name, e.g. 'content-type'.
   *
   * \return HTTP request header field value, if present.
   */
  std::optional<std::string_view> operator[](
      const std::string&) const noexcept override;

 protected:
  static constexpr std::string_view kContentLengthKey{"content-lenght"};

  // Mapping ftom HTTP request header field name to value.
  using HttpHeaderFields = std::unordered_map<std::string, std::string_view>;

  /// Mapping from string representation of HTTP method to the decoded value.
  static const std::unordered_map<std::string_view, HttpMethod> kMethodMap;

  bool valid_;                            //*!< Is HTTP request valid?
  HttpMethod method_;                     //*!< HTTP request method
  std::string uri_;                       //*!< HTTP URI
  std::optional<HttpResource> resource_;  //*!< HTTP resource (if present)
  HttpHeaderFields header_fields_;        //*!< HTTP header fields.

 private:
  void parseRequestLine(const std::vector<std::string_view>& lines);
  void parseHeaderFields(const std::vector<std::string_view>& lines);
};

}  // namespace request
}  // namespace http
}  // namespace protocol

#endif  // PROTOCOL_HTTP_REQUEST_SRC_HTTP_PARSER_HPP
