#ifndef PROTOCOL_IMEMBER_ACCESS_HPP
#define PROTOCOL_IMEMBER_ACCESS_HPP

#include <optional>
#include <string>

namespace protocol {

/**
 * \brief Member access interface.
 *
 * Interface for accessing members of a class using the "[]" operator.
 */
class IMemberAccess {
 public:
  virtual ~IMemberAccess() = default;
  /**
   * \brief Access object member with the specified key.
   *
   * \param key Key to identify the object member.
   *
   * \return Object member value if key is exists.
   */
  virtual std::optional<std::string_view> operator[](
      const std::string& key) const noexcept = 0;
};

}  // namespace protocol

#endif  // PROTOCOL_IMEMBER_ACCESS_HPP
