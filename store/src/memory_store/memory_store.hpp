#ifndef STORE_SRC_MEMORY_STORE_MEMORY_STORE_HPP
#define STORE_SRC_MEMORY_STORE_MEMORY_STORE_HPP

#include <unordered_map>

#include "store/file_store.hpp"

namespace store {

/**
 * \brief Filesystem representation.
 *
 * The mapping from filepaths to files is flat because there is no requirement
 * to support traversing the filesystem.
 *
 * A tree-like stucture is not needed if we only need to list all files stored
 * under the root directory.
 *
 * Additionally, there is no requirement on the order of files returned.
 */
using FileSystem = std::unordered_map<std::string, File>;

/**
 * \brief In-memory file store.
 *
 * \par Non-persistent file store.
 */
class MemoryStore : public FileStore {
 public:
  MemoryStore() = default;

  // MemoryStore is non-copyable and non-moveable because the semantics of
  // these operations would not be trivial. Moveover, there is no req to allow
  // for these operations and copying entire memory stores would have been
  // expensive.
  MemoryStore(const MemoryStore& other) = delete;
  MemoryStore(MemoryStore&& other) = delete;
  MemoryStore& operator=(const MemoryStore& other) = delete;
  MemoryStore& operator=(MemoryStore&&) = delete;

  std::pair<Result, const File&> get(
      const std::string& path) const noexcept override;
  Result put(const std::string& path, const File& file) noexcept override;
  FileList list() const noexcept override;
  Result remove(const std::string& path) noexcept override;

 protected:
  /**
   * \brief Check if a file with the given path exists.
   *
   * \param path Path at which to check.
   *
   * \return True if file exists, false otherwise.
   */
  bool exists(const std::string& path) const;

  FileSystem fs_;  //*!< Mapping from paths to files.
};

}  // namespace store

#endif  // STORE_SRC_MEMORY_STORE_MEMORY_STORE_HPP
