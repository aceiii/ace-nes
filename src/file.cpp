#include <filesystem>
#include <fstream>
#include <iterator>

#include "file.hpp"

using file::Error;


namespace {
  Error MapErrno(int error) {
    switch (error) {
      case ENOENT: return Error::FileNotFound;
      case EACCES: return Error::PermissionDenied;
      default: return Error::Unknown;
    }
  }
}


file::Result<Buffer> file::LoadBytes(std::string_view path) {
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

file::Result<std::vector<std::string>> file::ReadLines(std::string_view path) {
  std::ifstream input(std::string{path}, std::ios::in);
  if (input.fail()) {
    return std::unexpected{MapErrno(errno)};
  }

  std::vector<std::string> lines;
  std::string line;

  while (std::getline(input, line)) {
    lines.push_back(line);
  }

  return lines;
}
