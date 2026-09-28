#include "app.hpp"
#include "gui_app.hpp"
#include "headless_app.hpp"


std::unique_ptr<App> App::CreateApplication(const AppOptions &options) {
  if (options.headless) {
    return std::make_unique<HeadlessApp>(options);
  }
  return std::make_unique<GuiApp>(options);
}
