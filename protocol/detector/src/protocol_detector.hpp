#ifndef PROTOCOL_DETECTOR_SRC_PROTOCOL_DETECTOR_HPP
#define PROTOCOL_DETECTOR_SRC_PROTOCOL_DETECTOR_HPP

#include <string>

namespace protocol {

/**
 * \brief Internet application layer protocols supported by the detector.
 */
enum class AppLayerProtocol {
  Http,  //*!< HTTP
  Ftp,   //*!< FTP
};

/**
 * \brief Internet application layer protocol detector.
 */
class ProtocolDetector {
 public:
  /**
   * \brief Create internet application-layer protocol detector.
   *
   * \warning This implementation classifies every non-HTTP packet as FTP.
   *
   * \param buffer Input application-layer buffer.
   */
  ProtocolDetector(const std::string& buffer);

  // For now, I don't see the need for providing copy and move semantics
  ProtocolDetector(const ProtocolDetector& other) = delete;
  ProtocolDetector(ProtocolDetector&& other) = delete;
  ProtocolDetector& operator=(const ProtocolDetector& other) = delete;
  ProtocolDetector& operator=(ProtocolDetector&&) = delete;

  /**
   * \brief Detect application layer protocol.
   *
   * \return Applicaiton layer protocol used.
   */
  inline AppLayerProtocol detect() const { return protocol_; }

 protected:
  AppLayerProtocol protocol_;
};

}  // namespace protocol

#endif  // PROTOCOL_DETECTOR_SRC_PROTOCOL_DETECTOR_HPP
