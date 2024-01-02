#ifndef PROTOCOL_IRESPONSE_HPP
#define PROTOCOL_IRESPONSE_HPP

#include <string>

namespace protocol {

/**
 * \brief Response message interface.
 */
class IResponse {
 public:
  virtual ~IResponse() = default;

  /**
   * \brief Convert response to string.
   *
   * \return String representation of response
   */
  virtual operator std::string() const = 0;
};

}  // namespace protocol

#endif  // PROTOCOL_IRESPONSE_HPP
