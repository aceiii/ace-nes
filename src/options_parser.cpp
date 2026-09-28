#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "options_parser.hpp"


std::expected<AppOptions, std::string> OptionsParser::ParseOptions(std::span<char*> args) {
  argparse::ArgumentParser program("acenes", "0.0.1");

  program.add_argument("--log-level")
    .help("Set the verbosity for logging")
    .default_value("info")
    .choices("trace", "debug", "info", "warn", "err", "critical", "off")
    .nargs(1);

  program.add_argument("--headless")
    .help("Headless mode, path to NES ROM file");

  try {
    program.parse_args(args.size(), args.data());
  } catch (const std::exception &err) {
    return std::unexpected(err.what());
  }

  auto level_name = program.get("--log-level");
  auto log_level = magic_enum::enum_cast<spdlog::level::level_enum>(level_name);

  std::string rom_path;
  bool headless = false;
  if (program.is_used("--headless")) {
    headless = true;
    rom_path = program.get("--headless");
  }

  return AppOptions{
    .log_level = log_level.value_or(spdlog::level::info),
    .headless = headless,
    .rom_path = rom_path,
  };
}
