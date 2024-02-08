#include <boost/log/trivial.hpp>

#include "protocol/http/response/src/http_response.hpp"
#include "session.hpp"

namespace server {
namespace object_storage {

using protocol::http::request::HttpParser;
using protocol::http::response::HttpResource;
using protocol::http::response::HttpResponse;
using protocol::http::response::HttpResponseHeaders;
using protocol::http::response::HttpStatus;

void Session::handleHttp(std::string& request) noexcept {
  auto status_line_size = request.size();

  // Read the rest of the HTTP request (excluding message body)
  auto response_headers_size =
      boost::asio::read_until(socket_, input_stream_, "\r\n\r\n");
  std::istream stream(&input_stream_);
  request.resize(request.size() + response_headers_size);
  stream.read(&request[status_line_size], response_headers_size);

  BOOST_LOG_TRIVIAL(debug) << "HTTP request:\n" << request;

  // If request is valid, delegate it to the right handler based on the HTTP
  // method.
  HttpParser parser{request};
  if (!parser.isValid()) {
    BOOST_LOG_TRIVIAL(error) << "Failed to parse HTTP request:\n" << request;
    sendMessage(static_cast<std::string>(HttpResponse{HttpStatus::BadRequest}));

  } else if (!authHttpUser(parser)) {
    sendMessage(static_cast<std::string>(
        HttpResponse{HttpStatus::Unauthorized,
                     HttpResponseHeaders{{"WWW-Authenticate", "Basic"},
                                         {"Content-Length", "0"}}}));
  } else {
    http_handlers_.at(parser.getMethod())(parser);
  }
}

bool Session::authHttpUser(const HttpParser& parser) noexcept {
  if (authenticate_) {
    const auto auth_info = parser.getAuthInfo();
    user::User user_to_auth{auth_info->username, auth_info->password};
    const auto user_exists = user_database_.verify(user_to_auth);

    if (!user_exists) {
      BOOST_LOG_TRIVIAL(error)
          << "Failed to authenticate user " << user_to_auth;
    }

    return user_exists;
  }

  return true;
}

void Session::handleHttpGet(const HttpParser& parser) {
  if (parser.getUri() == "/") {
    // If request has 'GET /' format, list all files stored in the filesystem.
    const auto filepaths = filesystem_.list();
    std::string response;
    for (const auto& filepath : filepaths) {
      response += filepath + '\n';
    }
    sendMessage(
        static_cast<std::string>(HttpResponse{HttpStatus::Ok, response}));

  } else {
    // Otherwise, get the file from the filesystem and send it in response (if
    // it was found).
    const auto [status, file] = filesystem_.get(std::string{parser.getUri()});
    switch (status) {
      case fs::Status::Success:
        sendMessage(
            static_cast<std::string>(HttpResponse{HttpStatus::Ok, file}));
        break;
      case fs::Status::FileNotFound:
        sendMessage(
            static_cast<std::string>(HttpResponse{HttpStatus::NotFound}));
        break;
      default:
        sendMessage(static_cast<std::string>(
            HttpResponse{HttpStatus::InternalServerError}));
        break;
    }
  }

  receiveMessage();
}

void Session::handleHttpPut(const HttpParser& parser) {
  if (parser["expect"] && (*parser["expect"] == "100-continue")) {
    sendMessage(static_cast<std::string>(HttpResponse{HttpStatus::Continue}));
  }

  const auto file_size = parser.getResourceSize();
  const auto filepath = std::string{parser.getUri()};
  auto file = std::make_shared<fs::File>();
  file->resize(file_size);

  boost::asio::async_read(
      socket_, boost::asio::buffer(*file),
      boost::asio::transfer_exactly(file_size),
      serializer_.wrap([me = shared_from_this(), file, filepath](
                           ErrorCode error_code, std::size_t length) {
        if (error_code) {
          me->sendMessage(static_cast<std::string>(
              HttpResponse{HttpStatus::InternalServerError}));
        } else {
          const auto status = me->filesystem_.add(std::string{filepath}, *file);
          switch (status) {
            case fs::Status::Success:
              BOOST_LOG_TRIVIAL(info) << "Saved file: " << filepath;
              me->sendMessage(
                  static_cast<std::string>(HttpResponse{HttpStatus::Created}));
              break;
            case fs::Status::AlreadyExists:
              me->sendMessage(
                  static_cast<std::string>(HttpResponse{HttpStatus::NotFound}));
              break;
            default:
              me->sendMessage(static_cast<std::string>(
                  HttpResponse{HttpStatus::InternalServerError}));
              break;
          }
        }

        me->receiveMessage();
      }));
}

void Session::handleHttpDelete(const HttpParser& parser) {
  const auto& filepath = std::string{parser.getUri()};
  const auto status = filesystem_.remove(filepath);
  switch (status) {
    case fs::Status::Success:
      BOOST_LOG_TRIVIAL(info) << "Deleted file: " << filepath;
      sendMessage(static_cast<std::string>(HttpResponse{HttpStatus::Ok}));
      break;
    case fs::Status::FileNotFound:
      sendMessage(static_cast<std::string>(HttpResponse{HttpStatus::NotFound}));
      break;
    default:
      sendMessage(static_cast<std::string>(
          HttpResponse{HttpStatus::InternalServerError}));
      break;
  }

  receiveMessage();
}

}  // namespace object_storage
}  // namespace server