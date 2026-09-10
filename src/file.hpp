#pragma once

#include <expected>
#include <string_view>
#include <vector>

#include "types.hpp"


namespace file {

  using Buffer = std::vector<u8>;

  enum class FileError {
    Unknown,
    FileNotFound,
    PermissionDenied,
  };

  std::expected<Buffer, FileError> LoadBytes(std::string_view path);

}
