#include <atomic>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "headless_app.hpp"
#include "cart.hpp"

HeadlessApp::HeadlessApp(const AppOptions &options) : options_{options} {
}

int HeadlessApp::Run() {
  spdlog::info("Running in Headless mode");

  spdlog::set_level(options_.log_level);
  spdlog::trace("Log level was set to {}", magic_enum::enum_name(options_.log_level));

  spdlog::info("Loading ROM: {}", options_.rom_path);

  Cart cart;
  if (!cart.Load(options_.rom_path)) {
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
