#include "app.hpp"
#include "gui_app.hpp"


std::unique_ptr<App> App::CreateApplication(const AppOptions &options) {
  return std::make_unique<GuiApp>(options);
}
