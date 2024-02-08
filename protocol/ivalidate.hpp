#ifndef PROTOCOL_IVALIDATE_HPP
#define PROTOCOL_IVALIDATE_HPP

#include <string>

namespace protocol {

/**
 * \brief Validation interface.
 *
 * Interface for validating protocol packets.
 */
class IValidate {
 public:
  virtual ~IValidate() = default;

  /**
   * \brief Return whether a packet implements the intended protocol correctly.
   *
   * \return True if packet is valid, false otherwise.
   */
  [[nodiscard]] virtual bool isValid() const = 0;
};

}  // namespace protocol

#endif  // PROTOCOL_IVALIDATE_HPP
