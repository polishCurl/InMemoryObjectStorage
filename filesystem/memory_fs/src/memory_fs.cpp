#include "memory_fs.hpp"

using namespace fs;

std::pair<Status, const File&> MemoryFs::get(
    const std::string& path) const noexcept {
  std::shared_lock lock(mutex_);

  if (!exists(path)) {
    return {Status::FileNotFound, {}};
  }

  return {Status::Success, fs_.at(path)};
}

Status MemoryFs::add(const std::string& path, const File& file) noexcept {
  std::unique_lock lock(mutex_);

  if (exists(path)) {
    return Status::AlreadyExists;
  }

  fs_[path] = file;
  return Status::Success;
}

FileList MemoryFs::list() const noexcept {
  FileList list;
  std::shared_lock lock(mutex_);

  for (const auto& file : fs_) {
    list.emplace_back(file.first);
  }

  return list;
}

Status MemoryFs::remove(const std::string& path) noexcept {
  std::unique_lock lock(mutex_);
  return fs_.erase(path) == 1 ? Status::Success : Status::FileNotFound;
}

bool MemoryFs::exists(const std::string& path) const {
  return fs_.find(path) != fs_.end();
}
