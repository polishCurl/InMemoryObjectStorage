#include "memory_store.hpp"

using namespace store;

std::pair<Result, const File&> MemoryStore::get(
    const std::string& path) const noexcept {
  if (!exists(path)) {
    return {Result::kFileNotFound, {}};
  }

  return {Result::kSuccess, fs_.at(path)};
}

Result MemoryStore::put(const std::string& path, const File& file) noexcept {
  if (exists(path)) {
    return Result::kAlreadyExists;
  }

  fs_[path] = file;
  return Result::kSuccess;
}

FileList MemoryStore::list() const noexcept {
  FileList list;

  for (const auto& file : fs_) {
    list.emplace_back(file.first);
  }

  return list;
}

Result MemoryStore::remove(const std::string& path) noexcept {
  return fs_.erase(path) == 1 ? Result::kSuccess : Result::kFileNotFound;
}

bool MemoryStore::exists(const std::string& path) const {
  return fs_.find(path) != fs_.end();
}
