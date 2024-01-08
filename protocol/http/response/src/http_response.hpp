#ifndef PROTOCOL_HTTP_RESPONSE_SRC_HTTP_RESPONSE_HPP
#define PROTOCOL_HTTP_RESPONSE_SRC_HTTP_RESPONSE_HPP

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "protocol/iserialize.hpp"

namespace protocol {

namespace http {

namespace response {

/**
 * \brief HTTP status codes.
 */
enum class HttpStatus {
  /*####### 1xx - Informational #######*/
  Continue = 100,
  SwitchingProtocols = 101,
  Processing = 102,
  EarlyHints = 103,

  /*####### 2xx - Successful #######*/
  Ok = 200,
  Created = 201,
  Accepted = 202,
  NonAuthoritativeInformation = 203,
  NoContent = 204,
  ResetContent = 205,
  PartialContent = 206,
  MultiStatus = 207,
  AlreadyReported = 208,
  IMUsed = 226,

  /*####### 3xx - Redirection #######*/
  MultipleChoices = 300,
  MovedPermanently = 301,
  Found = 302,
  SeeOther = 303,
  NotModified = 304,
  UseProxy = 305,
  TemporaryRedirect = 307,
  PermanentRedirect = 308,

  /*####### 4xx - Client Error #######*/
  BadRequest = 400,
  Unauthorized = 401,
  PaymentRequired = 402,
  Forbidden = 403,
  NotFound = 404,
  MethodNotAllowed = 405,
  NotAcceptable = 406,
  ProxyAuthenticationRequired = 407,
  RequestTimeout = 408,
  Conflict = 409,
  Gone = 410,
  LengthRequired = 411,
  PreconditionFailed = 412,
  ContentTooLarge = 413,
  PayloadTooLarge = 413,
  URITooLong = 414,
  UnsupportedMediaType = 415,
  RangeNotSatisfiable = 416,
  ExpectationFailed = 417,
  ImATeapot = 418,
  MisdirectedRequest = 421,
  UnprocessableContent = 422,
  Locked = 423,
  FailedDependency = 424,
  TooEarly = 425,
  UpgradeRequired = 426,
  PreconditionRequired = 428,
  TooManyRequests = 429,
  RequestHeaderFieldsTooLarge = 431,
  UnavailableForLegalReasons = 451,

  /*####### 5xx - Server Error #######*/
  InternalServerError = 500,
  NotImplemented = 501,
  BadGateway = 502,
  ServiceUnavailable = 503,
  GatewayTimeout = 504,
  HttpVersionNotSupported = 505,
  VariantAlsoNegotiates = 506,
  InsufficientStorage = 507,
  LoopDetected = 508,
  NotExtended = 510,
  NetworkAuthenticationRequired = 511,
};

/**
 * \brief HTTP resource representation.
 */
using HttpResource = std::string;

/**
 * \brief HTTP response header (name and value).
 */
using HttpResponseHeader = std::pair<std::string, std::string>;

/**
 * \brief List of HTTP response headers.
 */
using HttpResponseHeaders = std::vector<HttpResponseHeader>;

/**
 * \brief HTTP response.
 */
class HttpResponse : public ISerialize {
 public:
  /**
   * \brief Create HTTP response with the given status.
   *
   * \param status HTTP status.
   */
  HttpResponse(HttpStatus status) noexcept;

  /**
   * \brief Create HTTP response with the given status and reason phrase.
   *
   * \param reason_phrase HTTP reason phrase.
   * \param status HTTP status
   */
  HttpResponse(const std::string& reason_phrase, HttpStatus status) noexcept;

  /**
   * \brief Create HTTP response with the given status and resource.
   *
   * \param status HTTP status.
   * \param resource HTTP resource.
   */
  HttpResponse(HttpStatus status, const HttpResource& resource) noexcept;

  /**
   * \brief Create HTTP response with the given status and resource.
   *
   * \param status HTTP status.
   * \param response_headers HTTP response headers.
   */
  HttpResponse(HttpStatus status,
               const HttpResponseHeaders& response_headers) noexcept;

  /**
   * \brief Create HTTP response with the given arguments.
   *
   * \param status HTTP status.
   * \param reason_phrase HTTP reason phrase.
   * \param response_headers HTTP response headers.
   * \param resource HTTP resource.
   */
  HttpResponse(HttpStatus status, const std::string& reason_phrase,
               const HttpResponseHeaders& response_headers,
               const HttpResource& resource) noexcept;

  /**
   * \brief Convert HTTP response to string.
   *
   * \return String representation of HTTP response.
   */
  operator std::string() const override;

 private:
  /// HTTP version used.
  static constexpr std::string_view kHttpVersion{"HTTP/1.1"};

  /// HTTP end-of-line marker.
  static constexpr std::string_view kEndOfLineMarker{"\r\n"};

  /// HTTP response header delimeter.
  static constexpr std::string_view kResponseHeaderDelim{": "};

  /// Response header name for specifying message body size.
  static constexpr std::string_view kContentLength{"Content-Length"};

  /// Default response header name for message body content type.
  static constexpr std::string_view kContentType{"Content-Type"};

  /// Default response header value for message body content type.
  static constexpr std::string_view kContentTypeValue{
      "application/octet-stream"};

  /// Mapping from HTTP status to the predefined reason phrase.
  static const std::unordered_map<HttpStatus, std::string> kStatusToReason;

  /// Initial output string buffer size for serializing the HTTP request (to
  /// avoid excessive resizing and copies).
  static constexpr std::size_t kInitialStrBufferSize{100};

  const HttpStatus status_;               ///< HTTP status.
  const std::string reason_phrase_;       ///< HTTP reason phrase.
  HttpResponseHeaders response_headers_;  ///< HTTP response headers.
  const HttpResource resource_;           ///< HTTP resource.
};

}  // namespace response
}  // namespace http
}  // namespace protocol

#endif  // PROTOCOL_HTTP_RESPONSE_SRC_HTTP_RESPONSE_HPP
