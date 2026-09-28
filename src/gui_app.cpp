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
  spdlog::info("Running in GUI mode");
  spdlog::set_level(options_.log_level);
  spdlog::trace("Log level was set to {}", magic_enum::enum_name(options_.log_level));

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
