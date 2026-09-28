#pragma once

#include "app.hpp"
#include "app_options.hpp"


class GuiApp final : public App {
public:
  GuiApp(const AppOptions& options);

  int Run() override;

private:
  bool Init();
  void MainLoop();
  void Update();
  void Cleanup();

private:
  AppOptions options_;
};
