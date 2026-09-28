#pragma once

#include <string>
#include <spdlog/spdlog.h>


struct AppOptions {
  spdlog::level::level_enum log_level;
  bool headless;
  std::string rom_path;
};
