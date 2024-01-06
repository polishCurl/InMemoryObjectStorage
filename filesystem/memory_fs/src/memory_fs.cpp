#include "memory_fs.hpp"

using namespace fs;

std::pair<Result, const File&> MemoryFs::get(
    std::string_view path) const noexcept {
  std::shared_lock lock(mutex_);

  if (!exists(path)) {
    return {Result::FileNotFound, {}};
  }

  return {Result::Success, fs_.at(path)};
}

Result MemoryFs::add(std::string_view path, const File& file) noexcept {
  std::unique_lock lock(mutex_);

  if (exists(path)) {
    return Result::AlreadyExists;
  }

  fs_[path] = file;
  return Result::Success;
}

FileList MemoryFs::list() const noexcept {
  FileList list;
  std::shared_lock lock(mutex_);

  for (const auto& file : fs_) {
    list.emplace_back(file.first);
  }

  return list;
}

Result MemoryFs::remove(std::string_view path) noexcept {
  std::unique_lock lock(mutex_);
  return fs_.erase(path) == 1 ? Result::Success : Result::FileNotFound;
}

bool MemoryFs::exists(std::string_view path) const {
  return fs_.find(path) != fs_.end();
}
