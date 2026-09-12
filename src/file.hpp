#pragma once

#include <expected>
#include <string_view>

#include "buffer.hpp"
#include "types.hpp"

namespace file {

  enum class Error {
    Unknown,
    FileNotFound,
    PermissionDenied
  };

  template <typename T>
  using Result = std::expected<T, Error>;

  using LoadResult = Result<Buffer>;

  LoadResult LoadBytes(std::string_view path);
}
