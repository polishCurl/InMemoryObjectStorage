#ifndef PROTOCOL_DETECTOR_SRC_PROTOCOL_DETECTOR_HPP
#define PROTOCOL_DETECTOR_SRC_PROTOCOL_DETECTOR_HPP

#include <string>

namespace protocol {

namespace detector {

/**
 * \brief Internet application layer protocols.
 */
enum class AppLayerProtocol {
  Http,  ///< HTTP
  Ftp,   ///< FTP
};

/**
 * \brief Detect internet application layer protocol from the input buffer.
 *
 * \attention This implementation classifies every non-HTTP packet as FTP.
 *
 * \param buffer Input internet application-layer buffer.
 *
 * \return Application layer protocol used.
 */
AppLayerProtocol detectProtocol(const std::string& buffer);

}  // namespace detector
}  // namespace protocol

#endif  // PROTOCOL_DETECTOR_SRC_PROTOCOL_DETECTOR_HPP
