#pragma once

#include <expected>
#include <span>
#include <string>

#include "app_options.hpp"


namespace OptionsParser {
  std::expected<AppOptions, std::string> ParseOptions(std::span<char*> args);
}
