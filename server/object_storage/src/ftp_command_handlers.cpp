#include <boost/log/trivial.hpp>

#include "protocol/ftp/response/src/ftp_response.hpp"
#include "session.hpp"

namespace server {
namespace object_storage {

using protocol::ftp::request::FtpCommand;
using protocol::ftp::request::FtpParser;
using protocol::ftp::response::FtpReplyCode;
using protocol::ftp::response::FtpResponse;

using user::User;

void Session::handleFtpRequest(const std::string& request) noexcept {
  // If request is valid, delegate it to the right handler based on the FTP
  // command.
  FtpParser parser{request};
  if (parser.isValid()) {
    const auto ftp_command = parser.getCommand();
    ftp_handlers_.at(ftp_command)(parser);
    last_ftp_command_ = ftp_command;
  } else {
    sendMessage(FtpResponse(FtpReplyCode::SYNTAX_ERROR_UNRECOGNIZED_COMMAND));
  }
}

void Session::handleFtpUser(const protocol::ftp::request::FtpParser& parser) {
  // Store the username until the PASS command is received.
  logged_in_user_.reset();
  last_username_ = parser.getTokens()[1];
  sendMessage(
      FtpResponse(FtpReplyCode::USER_NAME_OK, "Please provide password"));
}

void Session::handleFtpPass(const protocol::ftp::request::FtpParser& parser) {
  if (last_ftp_command_ != FtpCommand::User) {
    sendMessage(FtpResponse(FtpReplyCode::COMMANDS_BAD_SEQUENCE,
                            "Please specify username first"));
  } else {
    // Store the username until the PASS command is received.
    const User user{last_username_, std::string{parser.getTokens()[1]}};
    if (user_database_.exists(user)) {
      logged_in_user_ = user;
      sendMessage(
          FtpResponse(FtpReplyCode::USER_LOGGED_IN, "Login successful"));
    } else {
      sendMessage(FtpResponse(FtpReplyCode::NOT_LOGGED_IN, "Failed to log in"));
    }
  }
}

void Session::handleFtpList(const protocol::ftp::request::FtpParser& parser) {
  if (!logged_in_user_) {
    sendMessage(FtpResponse(FtpReplyCode::NOT_LOGGED_IN, "Not logged in"));
    return;
  }

  sendMessage(FtpResponse(FtpReplyCode::FILE_STATUS_OK_OPENING_DATA_CONNECTION,
                          "Listing all objects stored"));

  // Serialize the file list into a single text file.
  const auto filepaths = filesystem_.list();
  auto file = std::make_shared<fs::File>();
  for (const auto& filepath : filepaths) {
    *file += filepath + '\n';
  }

  // Asynchronous put the file into the FTP data send queue.
  auto data_socket = std::make_shared<Socket>(io_service_);
  ftp_data_acceptor_.async_accept(
      *data_socket,
      ftp_data_serializer_.wrap(
          [data_socket, file, me = shared_from_this()](auto error_code) {
            if (error_code) {
              me->sendMessage(FtpResponse(
                  FtpReplyCode::TRANSFER_ABORTED,
                  "Data transfer aborted: " + error_code.message()));
              return;
            }

            me->ftp_data_socket_ = data_socket;
            me->enqueueFtpDataHandler(file, data_socket);
            me->enqueueFtpDataHandler({}, data_socket);
          }));
}

void Session::handleFtpRetr(const protocol::ftp::request::FtpParser& parser) {
  if (!logged_in_user_) {
    sendMessage(FtpResponse(FtpReplyCode::NOT_LOGGED_IN, "Not logged in"));
    return;
  }

  if (!ftp_data_acceptor_.is_open()) {
    sendMessage(FtpResponse(FtpReplyCode::ERROR_OPENING_DATA_CONNECTION,
                            "Error opening data connection"));
    return;
  }

  const std::string filepath{parser.getTokens()[1]};
  // Otherwise, get the file from the filesystem and send it in response (if
  // it was found).
  const auto [status, file] = filesystem_.get(filepath);
  switch (status) {
    case fs::Status::Success:
      sendMessage(
          FtpResponse(FtpReplyCode::FILE_STATUS_OK_OPENING_DATA_CONNECTION,
                      "Sending file"));
      break;
    default:
      sendMessage(FtpResponse(FtpReplyCode::ACTION_ABORTED_LOCAL_ERROR,
                              "Error opening file for transfer"));
      break;
  }
  /*
  if (!file) {
    sendFtpMessage(FtpReplyCode::ACTION_ABORTED_LOCAL_ERROR,
                   "Error opening file for transfer");
    return;
  }

  sendFtpMessage(FtpReplyCode::FILE_STATUS_OK_OPENING_DATA_CONNECTION,
                 "Sending file");
  sendFile(file);
  */
}

void Session::handleFtpStor(const protocol::ftp::request::FtpParser& parser) {
  if (!logged_in_user_) {
    sendMessage(FtpResponse(FtpReplyCode::NOT_LOGGED_IN, "Not logged in"));
    return;
  }

  if (!ftp_data_acceptor_.is_open()) {
    sendMessage(FtpResponse(FtpReplyCode::ERROR_OPENING_DATA_CONNECTION,
                            "Error opening data connection"));
    return;
  }

  auto filepath = std::make_shared<std::string>(parser.getTokens()[1]);

  sendMessage(FtpResponse{FtpReplyCode::FILE_STATUS_OK_OPENING_DATA_CONNECTION,
                          "Ready to receive"});

  auto file = std::make_shared<fs::File>();
  receiveFtpDataHandler(file, filepath);
}

void Session::handleFtpDele(const protocol::ftp::request::FtpParser& parser) {
  if (!logged_in_user_) {
    sendMessage(FtpResponse(FtpReplyCode::NOT_LOGGED_IN, "Not logged in"));
    return;
  }

  const auto& filepath = std::string{parser.getTokens()[1]};
  const auto status = filesystem_.remove(filepath);
  switch (status) {
    case fs::Status::Success:
      BOOST_LOG_TRIVIAL(info) << "Deleted file: " << filepath;
      sendMessage(
          FtpResponse(FtpReplyCode::FILE_ACTION_COMPLETED, "File deleted"));
      break;
    default:
      sendMessage(
          FtpResponse(FtpReplyCode::ACTION_NOT_TAKEN, "Unable to delete file"));

      break;
  }
}

void Session::handleFtpPasv(const protocol::ftp::request::FtpParser& parser) {
  if (!logged_in_user_) {
    sendMessage(FtpResponse(FtpReplyCode::NOT_LOGGED_IN, "Not logged in"));
    return;
  }

  if (!setUpFtpDataConnectionAcceptor()) {
    sendMessage(FtpResponse(FtpReplyCode::SERVICE_NOT_AVAILABLE,
                            "Passive mode not supported"));
  } else {
    // Respond to FTP client with IP address and port for data connection.
    const auto ip_bytes = socket_.local_endpoint().address().to_v4().to_bytes();
    const auto port = ftp_data_acceptor_.local_endpoint().port();

    std::stringstream stream;
    stream << "(";
    for (const auto byte : ip_bytes) {
      stream << static_cast<unsigned int>(byte) << ",";
    }
    stream << ((port >> 8) & 0xff) << "," << (port & 0xff) << ")";
    sendMessage(FtpResponse(FtpReplyCode::ENTERING_PASSIVE_MODE,
                            "Entering passive mode " + stream.str()));
  }
}

void Session::handleFtpType(const protocol::ftp::request::FtpParser& parser) {
  if (!logged_in_user_) {
    sendMessage(FtpResponse(FtpReplyCode::NOT_LOGGED_IN, "Not logged in"));
  } else {
    sendMessage(FtpResponse(FtpReplyCode::COMMAND_OK, "Mode switched"));
  }
}

}  // namespace object_storage
}  // namespace server