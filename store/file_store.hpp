#ifndef STORE_FILE_STORE_HPP
#define STORE_FILE_STORE_HPP

#include <string>
#include <vector>

namespace store {

/**
 * \brief File store operation result.
 */
enum class Result {
  kSuccess,        //*!< File store operation completed successfully.
  kFileNotFound,   //*!< Specified file was not found.
  kAlreadyExists,  //*!< File already exists at the specified path.
};

/**
 * \brief File representation.
 */
using File = std::vector<char>;  // std::vector guarantees contiguous memory

/**
 * \brief List of filenames (paths)
 */
using FileList = std::vector<std::string>;

/**
 * \brief File store interface
 */
class FileStore {
 public:
  virtual ~FileStore() = default;

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
   * \brief Put file at the specified path.
   *
   * \param path Path at which to put the file.
   * \param file File to put.
   *
   * \return Result of the put operation.
   */
  virtual Result put(const std::string& path, const File& file) noexcept = 0;

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

}  // namespace store

#endif  // STORE_FILE_STORE_HPP
