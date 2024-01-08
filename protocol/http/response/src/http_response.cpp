#include "http_response.hpp"

namespace protocol {

namespace http {

namespace response {

HttpResponse::HttpResponse(HttpStatus status) noexcept

    : HttpResponse{status,
                   // Assign the default reason phrase from the HTTP status.
                   kStatusToReason.at(status),
                   // Even if we only send status code (no resource), curl still
                   // expects the Content-Length to be set to 0.
                   {HttpResponseHeader{kContentLength, "0"}},
                   {}} {}

HttpResponse::HttpResponse(const std::string& reason_phrase,
                           HttpStatus status) noexcept
    : HttpResponse{status,
                   reason_phrase,
                   {HttpResponseHeader{kContentLength, "0"}},
                   {}} {}

HttpResponse::HttpResponse(HttpStatus status,
                           const HttpResource& resource) noexcept
    : HttpResponse{status,
                   kStatusToReason.at(status),
                   // Since we are passing resource, we need to provide content
                   // type and length in HTTP resource headers.
                   {{HttpResponseHeader{kContentType, kContentTypeValue},
                     HttpResponseHeader{kContentLength,
                                        std::to_string(resource.size())}}},
                   resource} {}

HttpResponse::HttpResponse(HttpStatus status,
                           const HttpResponseHeaders& response_headers) noexcept
    : HttpResponse{status, kStatusToReason.at(status), response_headers, {}} {}

HttpResponse::HttpResponse(HttpStatus status, const std::string& reason_phrase,
                           const HttpResponseHeaders& response_headers,
                           const HttpResource& resource) noexcept
    : status_{status},
      reason_phrase_{reason_phrase},
      response_headers_{response_headers},
      resource_{resource} {};

HttpResponse::operator std::string() const {
  std::string str;
  str.reserve(kInitialStrBufferSize);

  // Status line
  str += kHttpVersion;
  str += ' ';
  str += std::to_string(static_cast<int>(status_));
  str += ' ';
  str += reason_phrase_;
  str += kEndOfLineMarker;

  // Response headers
  for (const auto& response_header : response_headers_) {
    str += response_header.first;
    str += kResponseHeaderDelim;
    str += response_header.second;
    str += kEndOfLineMarker;
  }

  str += kEndOfLineMarker;

  // Message body
  str += resource_;

  return str;
}

const std::unordered_map<HttpStatus, std::string> HttpResponse::kStatusToReason{

    // 1xx - Informational
    {HttpStatus::Continue, "Continue"},
    {HttpStatus::SwitchingProtocols, "Switching Protocols"},
    {HttpStatus::Processing, "Processing"},
    {HttpStatus::EarlyHints, "Early Hints"},

    // 2xx - Successful
    {HttpStatus::Ok, "OK"},
    {HttpStatus::Created, "Created"},
    {HttpStatus::Accepted, "Accepted"},
    {HttpStatus::NonAuthoritativeInformation, "Non-Authoritative Information"},
    {HttpStatus::NoContent, "No Content"},
    {HttpStatus::ResetContent, "Reset Content"},
    {HttpStatus::PartialContent, "Partial Content"},
    {HttpStatus::MultiStatus, "Multi-Status"},
    {HttpStatus::AlreadyReported, "Already Reported"},
    {HttpStatus::IMUsed, "IM Used"},

    // 3xx - Redirection
    {HttpStatus::MultipleChoices, "Multiple Choices"},
    {HttpStatus::MovedPermanently, "Moved Permanently"},
    {HttpStatus::Found, "Found"},
    {HttpStatus::SeeOther, "See Other"},
    {HttpStatus::NotModified, "Not Modified"},
    {HttpStatus::UseProxy, "Use Proxy"},
    {HttpStatus::TemporaryRedirect, "Temporary Redirect"},
    {HttpStatus::PermanentRedirect, "Permanent Redirect"},

    // 4xx - Client Error
    {HttpStatus::BadRequest, "Bad Request"},
    {HttpStatus::Unauthorized, "Unauthorized"},
    {HttpStatus::PaymentRequired, "Payment Required"},
    {HttpStatus::Forbidden, "Forbidden"},
    {HttpStatus::NotFound, "Not Found"},
    {HttpStatus::MethodNotAllowed, "Method Not Allowed"},
    {HttpStatus::NotAcceptable, "Not Acceptable"},
    {HttpStatus::ProxyAuthenticationRequired, "Proxy Authentication Required"},
    {HttpStatus::RequestTimeout, "Request Timeout"},
    {HttpStatus::Conflict, "Conflict"},
    {HttpStatus::Gone, "Gone"},
    {HttpStatus::LengthRequired, "Length Required"},
    {HttpStatus::PreconditionFailed, "Precondition Failed"},
    {HttpStatus::ContentTooLarge, "Content Too Large"},
    {HttpStatus::URITooLong, "URI Too Long"},
    {HttpStatus::UnsupportedMediaType, "Unsupported Media Type"},
    {HttpStatus::RangeNotSatisfiable, "Range Not Satisfiable"},
    {HttpStatus::ExpectationFailed, "Expectation Failed"},
    {HttpStatus::ImATeapot, "I'm a teapot"},
    {HttpStatus::MisdirectedRequest, "Misdirected Request"},
    {HttpStatus::UnprocessableContent, "Unprocessable Content"},
    {HttpStatus::Locked, "Locked"},
    {HttpStatus::FailedDependency, "Failed Dependency"},
    {HttpStatus::TooEarly, "Too Early"},
    {HttpStatus::UpgradeRequired, "Upgrade Required"},
    {HttpStatus::PreconditionRequired, "Precondition Required"},
    {HttpStatus::TooManyRequests, "Too Many Requests"},
    {HttpStatus::RequestHeaderFieldsTooLarge,
     "Request Header Fields Too Large"},
    {HttpStatus::UnavailableForLegalReasons, "Unavailable For Legal Reasons"},

    // 5xx - Server Error
    {HttpStatus::InternalServerError, "Internal Server Error"},
    {HttpStatus::NotImplemented, "Not Implemented"},
    {HttpStatus::BadGateway, "Bad Gateway"},
    {HttpStatus::ServiceUnavailable, "Service Unavailable"},
    {HttpStatus::GatewayTimeout, "Gateway Timeout"},
    {HttpStatus::HttpVersionNotSupported, "HTTP Version Not Supported"},
    {HttpStatus::VariantAlsoNegotiates, "Variant Also Negotiates"},
    {HttpStatus::InsufficientStorage, "Insufficient Storage"},
    {HttpStatus::LoopDetected, "Loop Detected"},
    {HttpStatus::NotExtended, "Not Extended"},
    {HttpStatus::NetworkAuthenticationRequired,
     "Network Authentication Required"},
};

}  // namespace response
}  // namespace http
}  // namespace protocol