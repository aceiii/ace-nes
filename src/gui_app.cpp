#include <atomic>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include "gui_app.hpp"
#include "cart.hpp"

namespace {
  std::atomic<bool> g_quit{false};

  SDL_Window* g_window = nullptr;
  SDL_Renderer* g_renderer = nullptr;
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

  g_renderer = SDL_CreateRenderer(g_window, nullptr);
  if (!g_renderer) {
    spdlog::error("Failed to create renderer: {}", SDL_GetError());
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui_ImplSDL3_InitForSDLRenderer(g_window, g_renderer);
  ImGui_ImplSDLRenderer3_Init(g_renderer);

  return true;
}

void GuiApp::MainLoop() {
  spdlog::trace("Entering GuiApp main loop");

  g_quit = false;

  SDL_Event event;
  while (!g_quit) {
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) {
        g_quit = true;
      }
    }

    Update();
  }
}

void GuiApp::Update() {
  ImGui_ImplSDLRenderer3_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Hello, world");
  ImGui::Text("This is ImGui runnin on SDL3 without OpenGL");
  ImGui::End();

  ImGui::Render();
  SDL_SetRenderDrawColor(g_renderer, 196, 153, 122, 255);
  SDL_RenderClear(g_renderer);

  ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), g_renderer);
  SDL_RenderPresent(g_renderer);
}

void GuiApp::Cleanup() {
  spdlog::trace("Cleaning up GuiApp");

  ImGui_ImplSDLRenderer3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_DestroyWindow(g_window);
  SDL_Quit();
}
