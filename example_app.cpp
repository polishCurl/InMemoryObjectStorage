/**
 * \mainpage Object Storage Server source code documentation
 *
 * Object storage is an FTP/HTTP server for in-memory object storage and
 * retrieval.
 */

#include <iostream>

#include "server/object_storage/src/object_storage.hpp"

using namespace server::object_storage;

int main(int argc, char *argv[]) {
  // ObjectStorage server{"127.0.0.1", 1670, LogLevel::debug};
  ObjectStorage server{"127.0.0.1", 1670, LogLevel::debug, true};
  server.addUser("Nord", "VPN");

  if (!server.start()) {
    return -1;
  }

  std::cout << "Object storage server is running...\n";
  std::cout << "Press <Enter> to exit\n";
  std::cin.get();

  return 0;
}