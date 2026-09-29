#include <atomic>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>

#include "gui_app.hpp"
#include "cart.hpp"

namespace {
  std::atomic<bool> g_quit{false};

  SDL_Window* g_window = nullptr;
};

static bool SetLoggingLevel(const std::string &level_name) {
  auto level = magic_enum::enum_cast<spdlog::level::level_enum>(level_name);
  if (level.has_value()) {
    spdlog::set_level(level.value());
    return true;
  }
  return false;
}

GuiApp::GuiApp(const AppOptions &options) : options_{options} {
}

int GuiApp::Run() {
  spdlog::info("Running in GUI mode");
  spdlog::set_level(options_.log_level);
  spdlog::trace("Log level was set to {}", magic_enum::enum_name(options_.log_level));

  if (Init()) {
    MainLoop();
  }
  Cleanup();
  return 0;
}

bool GuiApp::Init() {
  spdlog::trace("Initializing GuiApp");

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    spdlog::error("Failed to initialize SDL: {}", SDL_GetError());
    return false;
  }

  g_window = SDL_CreateWindow("AceNES", 800, 600, 0);
  if (!g_window) {
    spdlog::error("Failed to create window: {}", SDL_GetError());
    return false;
  }

  return true;
}

void GuiApp::MainLoop() {
  spdlog::trace("Entering GuiApp main loop");

  g_quit = false;

  SDL_Event event;
  while (!g_quit) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_EVENT_QUIT) {
        g_quit = true;
      }
    }

    Update();
  }
}

void GuiApp::Update() {
}

void GuiApp::Cleanup() {
  spdlog::trace("Cleaning up GuiApp");

  SDL_DestroyWindow(g_window);
  SDL_Quit();
}
