#ifndef FILESYSTEM_MEMORY_FS_SRC_MEMORY_FS_HPP
#define FILESYSTEM_MEMORY_FS_SRC_MEMORY_FS_HPP

#include <unordered_map>

#include "filesystem/ifilesystem.hpp"

namespace fs {

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
using Fs = std::unordered_map<std::string, File>;

/**
 * \brief In-memory filesystem.
 *
 * \par Non-persistent filesystem.
 */
class MemoryFs : public IFilesystem {
 public:
  MemoryFs() = default;

  // MemoryFs is non-copyable and non-moveable because the semantics of
  // these operations would not be trivial. Moveover, there is no req to allow
  // for these operations and copying entire memory stores would have been
  // expensive.
  MemoryFs(const MemoryFs& other) = delete;
  MemoryFs(MemoryFs&& other) = delete;
  MemoryFs& operator=(const MemoryFs& other) = delete;
  MemoryFs& operator=(MemoryFs&&) = delete;

  std::pair<Result, const File&> get(
      const std::string& path) const noexcept override;
  Result add(const std::string& path, const File& file) noexcept override;
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

  Fs fs_;  //*!< Mapping from paths to files.
};

}  // namespace fs

#endif  // FILESYSTEM_MEMORY_FS_SRC_MEMORY_FS_HPP
