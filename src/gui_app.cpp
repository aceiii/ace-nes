#include <atomic>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "gui_app.hpp"
#include "cart.hpp"

namespace {
  std::atomic<bool> g_quit{false};
};

static bool SetLoggingLevel(const std::string &level_name) {
  auto level = magic_enum::enum_cast<spdlog::level::level_enum>(level_name);
  if (level.has_value()) {
    spdlog::set_level(level.value());
    return true;
  }
  return false;
}

GuiApp::GuiApp(const AppOptions &options) : options_{options}
{
}

int GuiApp::Run() {
  spdlog::set_level(options_.log_level);
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


  // if (Init()) {
  //   MainLoop();
  // }
  // Cleanup();
  return 0;
}

bool GuiApp::Init() {

  return true;
}

void GuiApp::MainLoop() {
  g_quit = false;
  while (g_quit) {
    Update();
  }
}

void GuiApp::Update() {
}

void GuiApp::Cleanup() {
}
