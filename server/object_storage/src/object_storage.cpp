#include "object_storage.hpp"

#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <iostream>

#include "session.hpp"

namespace server {

namespace object_storage {

ObjectStorage::ObjectStorage(const std::string& address, uint16_t port,
                             LogLevel log_level)
    : address_{address},
      port_{port},
      log_level_{log_level},
      acceptor_{io_service_},
      open_connection_count_{0} {
  setUpLogging();
}

bool ObjectStorage::start(std::size_t thread_count) {
  if (thread_count == 0) {
    return false;
  }

  if (!setUpSessionAcceptor()) {
    return false;
  }

  for (size_t i = 0; i < thread_count; i++) {
    workers_.emplace_back([this] { io_service_.run(); });
  }

  BOOST_LOG_TRIVIAL(info) << "Server listening at "
                          << acceptor_.local_endpoint().address() << ':'
                          << acceptor_.local_endpoint().port();

  return true;
}

void ObjectStorage::stop() {
  BOOST_LOG_TRIVIAL(info) << "Stopping server...";
  io_service_.stop();

  for (auto& thread : workers_) {
    thread.join();
  }
}

bool ObjectStorage::addUser(const std::string& username,
                            const std::string& password) noexcept {
  const auto user_added = users_.add({username, password});

  if (user_added) {
    BOOST_LOG_TRIVIAL(info)
        << "User added: '" << username << ':' << password << '\'';
  } else {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to add user: '" << username << ':' << password << '\'';
  }

  return user_added;
}

bool ObjectStorage::setUpSessionAcceptor() noexcept {
  ErrorCode error_code{};
  const Endpoint endpoint{boost::asio::ip::make_address(address_, error_code),
                          port_};

  if (error_code) {
    BOOST_LOG_TRIVIAL(error) << "Failed to create address from string \""
                             << address_ << "\": " << error_code.message();
    return false;
  }

  acceptor_.open(endpoint.protocol(), error_code);
  if (error_code) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to open acceptor: " << error_code.message();
    return false;
  }

  acceptor_.set_option(Acceptor::reuse_address(true), error_code);
  if (error_code) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to set reuse_address option: " << error_code.message();
    return false;
  }

  acceptor_.bind(endpoint, error_code);
  if (error_code) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to bind acceptor: " << error_code.message();
    return false;
  }

  acceptor_.listen(boost::asio::socket_base::max_listen_connections,
                   error_code);
  if (error_code) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to listen on acceptor: " << error_code.message();
    return false;
  }

  auto session = std::make_shared<Session>(
      io_service_, users_, filesystem_, [this]() { open_connection_count_--; });

  acceptor_.async_accept(session->getSocket(),
                         [this, session](auto error_code) {
                           open_connection_count_++;
                           acceptSession(session, error_code);
                         });

  return true;
}

void ObjectStorage::acceptSession(const std::shared_ptr<Session>& session,
                                  ErrorCode const& error_code) noexcept {
  if (error_code) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to accept session: " << error_code.message();
    return;
  }

  BOOST_LOG_TRIVIAL(debug)
      << "Client connected: "
      << session->getSocket().remote_endpoint().address().to_string() << ':'
      << session->getSocket().remote_endpoint().port();

  session->start();

  auto new_session = std::make_shared<Session>(
      io_service_, users_, filesystem_, [this]() { open_connection_count_--; });

  acceptor_.async_accept(new_session->getSocket(),
                         [this, new_session](auto error_code) {
                           open_connection_count_++;
                           acceptSession(new_session, error_code);
                         });
}

void ObjectStorage::setUpLogging() noexcept {
  boost::log::core::get()->set_filter(boost::log::trivial::severity >=
                                      static_cast<int>(log_level_));
}

}  // namespace object_storage
}  // namespace server
