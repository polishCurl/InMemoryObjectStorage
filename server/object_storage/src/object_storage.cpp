#include "object_storage.hpp"

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
    std::cerr << "Failed to create address from string \"" << address_
              << "\": " << ec.message() << '\n';
    return false;
  }

  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    std::cerr << "Failed to open acceptor: " << ec.message() << '\n';
    return false;
  }

  acceptor_.set_option(Acceptor::reuse_address(true), ec);
  if (ec) {
    std::cerr << "Failed to se reuse_address option: " << ec.message() << '\n';
    return false;
  }

  acceptor_.bind(endpoint, ec);
  if (ec) {
    std::cerr << "Error binding acceptor: " << ec.message() << '\n';
    return false;
  }

  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    std::cerr << "Error listening on acceptor: " << ec.message() << '\n';
    return false;
  }

  std::cout << "Object storage server listening at "
            << acceptor_.local_endpoint().address() << ':'
            << acceptor_.local_endpoint().port() << '\n';

  return true;
}

void ObjectStorage::stop() {}

}  // namespace object_storage
}  // namespace server
