#include <string>
#include <string_view>
#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "cart.hpp"
#include "cpu.hpp"

static bool set_logging_level(const std::string &level_name) {
  auto level = magic_enum::enum_cast<spdlog::level::level_enum>(level_name);
  if (level.has_value()) {
    spdlog::set_level(level.value());
    return true;
  }
  return false;
}

auto main(int argc, char *argv[]) -> int {
  spdlog::set_level(spdlog::level::info);

  argparse::ArgumentParser program("acenes", "0.0.1");

  program.add_argument("--log-level")
    .help("Set the verbosity for logging")
    .default_value(std::string("info"))
    .nargs(1);

  program.add_argument("rom")
    .default_value("./roms/nestest.nes")
    .help("path to nestest.nes ROM file");

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  const std::string level = program.get("--log-level");
  if (!set_logging_level(level)) {
    std::cerr << fmt::format("Invalid argument \"{}\" - allowed options: "
                             "{{trace, debug, info, warn, err, critical, off}}",
                             level)
              << std::endl;
    std::cerr << program;
    return 1;
  }

  const std::string rom_path = program.get("rom");

  spdlog::info("Loading ROM: {}", rom_path);

  Cart cart;
  if (!cart.Load(rom_path)) {
    spdlog::error("Failed to load ROM!");
    return 1;
  }

  spdlog::info("ROM Loaded!");

  const auto& header = cart.Header();

  spdlog::info("Ident: {}", std::string(header.ident.begin(), header.ident.end()));
  spdlog::info("Console Type: {}", magic_enum::enum_name(header.GetConsoleType()));
  spdlog::info("Video Format: {}", magic_enum::enum_name(header.GetVideoFormat()));
  spdlog::info("Has Battery: {}", header.HasBattery());
  spdlog::info("Has Trainer: {}", header.HasTrainer());

  spdlog::info("Exiting.");

  return 0;
}
