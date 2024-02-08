/**
 * \mainpage Object Storage Server source code documentation
 *
 * Object storage is an FTP/HTTP server for in-memory object storage and
 * retrieval.
 */

#include <cstdlib>
#include <iostream>
#include <string>

#include "server/object_storage/src/object_storage.hpp"
#include "utils/src/utils.hpp"

using namespace server::object_storage;

int main(int argc, char *argv[]) {
  // ObjectStorage server{"127.0.0.1", 1670, LogLevel::Debug};

  if (argc != 6) {
    std::cout << "Usage ./object_storage <address> <port> <threads> "
                 "<auth|no_auth> <ftp_port_range>\n";
    return -1;
  }

  // Parse command line arguments
  const std::string address = argv[1];            // NOLINT
  const std::uint16_t port = std::atoi(argv[2]);  // NOLINT
  const auto thread_count = std::atoi(argv[3]);   // NOLINT
  const auto authenticate =
      std::string{argv[4]} == std::string{"auth"};         // NOLINT
  const auto ftp_port_range = utils::split(argv[5], "-");  // NOLINT
  std::uint16_t ftp_port_min = std::atoi(ftp_port_range[0].data());
  std::uint16_t ftp_port_max = std::atoi(ftp_port_range[1].data());

  // Instantiate the Object storage server
  ObjectStorage server{
      address, port, LogLevel::Info, authenticate, {ftp_port_min, ftp_port_max},
  };

  // Add users (for authentication)
  server.addUser("Nord", "VPN");

  // Start the server with the given number of threads
  if (!server.start(thread_count)) {
    return -1;
  }

  std::cout << "Object storage server is running...\n";
  std::cout << "Press <Enter> to exit\n";
  std::cin.get();

  return 0;
}