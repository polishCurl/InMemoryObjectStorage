#include "protocol_detector.hpp"

namespace protocol {

ProtocolDetector::ProtocolDetector(const std::string& buffer) {
  const auto end_of_line = buffer.find("\r\n");
  std::string_view header{buffer.c_str(), end_of_line};
  const auto http_pos = header.find("HTTP");
  protocol_ = http_pos == std::string::npos ? AppLayerProtocol::Ftp
                                            : AppLayerProtocol::Http;
}

}  // namespace protocol
