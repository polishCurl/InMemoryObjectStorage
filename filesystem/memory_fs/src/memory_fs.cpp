#include "memory_fs.hpp"

using namespace fs;

std::pair<Result, const File&> MemoryFs::get(
    const std::string& path) const noexcept {
  if (!exists(path)) {
    return {Result::FileNotFound, {}};
  }

  return {Result::Success, fs_.at(path)};
}

Result MemoryFs::add(const std::string& path, const File& file) noexcept {
  if (exists(path)) {
    return Result::AlreadyExists;
  }

  fs_[path] = file;
  return Result::Success;
}

FileList MemoryFs::list() const noexcept {
  FileList list;

  for (const auto& file : fs_) {
    list.emplace_back(file.first);
  }

  return list;
}

Result MemoryFs::remove(const std::string& path) noexcept {
  return fs_.erase(path) == 1 ? Result::Success : Result::FileNotFound;
}

bool MemoryFs::exists(const std::string& path) const {
  return fs_.find(path) != fs_.end();
}
