#include "object_storage.hpp"

#include <boost/log/trivial.hpp>
#include <iostream>

using Acceptor = boost::asio::ip::tcp::acceptor;
using Endpoint = boost::asio::ip::tcp::endpoint;
using ErrorCode = boost::system::error_code;

namespace server {

namespace object_storage {

ObjectStorage::ObjectStorage(const std::string& address, uint16_t port)
    : address_{address},
      port_{port},
      acceptor_{io_service_},
      open_connection_count_{0} {}

bool ObjectStorage::start(std::size_t thread_count) {
  if (thread_count == 0) {
    return false;
  }

  // Set up the acceptor to listen on the tcp port
  ErrorCode ec{};
  const Endpoint endpoint{boost::asio::ip::make_address(address_, ec), port_};

  if (ec) {
    BOOST_LOG_TRIVIAL(error) << "Failed to create address from string \""
                             << address_ << "\": " << ec.message();
    return false;
  }

  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << "Failed to open acceptor: " << ec.message();
    return false;
  }

  acceptor_.set_option(Acceptor::reuse_address(true), ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to set reuse_address option: " << ec.message();
    return false;
  }

  acceptor_.bind(endpoint, ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(error) << "Failed to bind acceptor: " << ec.message();
    return false;
  }

  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    BOOST_LOG_TRIVIAL(error)
        << "Failed to listen on acceptor: " << ec.message();
    return false;
  }

  BOOST_LOG_TRIVIAL(info) << "Object storage server listening at "
                          << acceptor_.local_endpoint().address() << ':'
                          << acceptor_.local_endpoint().port();

  for (size_t i = 0; i < thread_count; i++) {
    thread_pool_.emplace_back([this] { io_service_.run(); });
  }

  return true;
}

void ObjectStorage::stop() {
  BOOST_LOG_TRIVIAL(info) << "Stopping server...";
  io_service_.stop();

  for (auto& thread : thread_pool_) {
    thread.join();
  }
}

bool ObjectStorage::addUser(const std::string& username,
                            const std::string& password) {
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

}  // namespace object_storage
}  // namespace server
