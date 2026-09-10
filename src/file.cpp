#include <filesystem>
#include <fstream>
#include <iterator>

#include "file.hpp"

using file::Buffer, file::FileError;


namespace {
  FileError MapErrno(int error) {
    switch (error) {
      case ENOENT: return FileError::FileNotFound;
      case EACCES: return FileError::PermissionDenied;
      default: return FileError::Unknown;
    }
  }
}


std::expected<Buffer, FileError> file::LoadBytes(std::string_view path) {
  std::ifstream input(std::string{path}, std::ios::in | std::ios::binary);
  if (input.fail()) {
    return std::unexpected{MapErrno(errno)};
  }

  input.unsetf(std::ios::skipws);

  input.seekg(0, std::ios::end);
  auto size = input.tellg();
  input.seekg(0, std::ios::beg);

  std::vector<u8> bytes;
  bytes.reserve(size);
  std::copy(std::istream_iterator<u8>(input), std::istream_iterator<u8>(), std::back_inserter(bytes));

  return bytes;
}
