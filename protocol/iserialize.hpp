#ifndef PROTOCOL_ISERIALIZE_HPP
#define PROTOCOL_ISERIALIZE_HPP

#include <string>

namespace protocol {

/**
 * \brief Serialization interface.
 *
 * Interface for serializing any object into a string.
 */
class ISerialize {
 public:
  virtual ~ISerialize() = default;

  /**
   * \brief Convert object to string.
   *
   * \return String representation of object.
   */
  virtual explicit operator std::string() const = 0;
};

}  // namespace protocol

#endif  // PROTOCOL_ISERIALIZE_HPP
