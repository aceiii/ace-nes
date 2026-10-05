#include <sstream>

#include "string.hpp"

std::string_view string::Trim(std::string_view input, std::string_view chars) {
  return TrimLeading(TrimTrailing(input, chars), chars);
}

std::string_view string::TrimLeading(std::string_view input, std::string_view chars) {
  const auto idx = input.find_first_not_of(chars);
  return std::string_view(input.begin() + idx, input.end());
}

std::string_view string::TrimTrailing(std::string_view input, std::string_view chars) {
  const auto idx = input.find_last_not_of(chars);
  return std::string_view(input.begin(), input.begin() + idx + 1);
}

std::string string::Join(const std::vector<std::string>& words, std::string_view delim) {
  if (words.empty()) {
    return "";
  }

  std::ostringstream ss;
  ss << words[0];

  for (auto idx = 1; idx < words.size(); ++idx) {
    ss << delim << words[idx];
  }

  return ss.str();
}
