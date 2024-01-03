#ifndef FILESYSTEM_FILESYSTEM_HPP
#define FILESYSTEM_FILESYSTEM_HPP

#include <string>
#include <vector>

namespace fs {

/**
 * \brief Filesystem operation result.
 */
enum class Result {
  Success,        //*!< Filesystem operation completed successfully.
  FileNotFound,   //*!< Specified file was not found.
  AlreadyExists,  //*!< File already exists at the specified path.
};

/**
 * \brief File representation.
 */
using File = std::string;

/**
 * \brief List of filenames (paths)
 */
using FileList = std::vector<std::string>;

/**
 * \brief Filesystem interface
 */
class IFilesystem {
 public:
  virtual ~IFilesystem() = default;

  /**
   * \brief Get file from the specified path.
   *
   * \param path Path to the file to get.
   *
   * \return Operation result and the file (if successfull).
   */
  virtual std::pair<Result, const File&> get(
      const std::string& path) const noexcept = 0;

  /**
   * \brief Add file at the specified path.
   *
   * \param path Path at which to add the file.
   * \param file File to add.
   *
   * \return Result of the add operation.
   */
  virtual Result add(const std::string& path, const File& file) noexcept = 0;

  /**
   * \brief List all stored objects.
   *
   * \return List of all objects stored in the filestore.
   */
  virtual FileList list() const noexcept = 0;

  /**
   * \brief Remove file from the specified path.
   *
   * \param path Path to the file to remove.
   *
   * \return Result of the remove operation.
   */
  virtual Result remove(const std::string& path) noexcept = 0;
};

}  // namespace fs

#endif  // FILESYSTEM_FILESYSTEM_HPP
