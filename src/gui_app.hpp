#pragma once

#include "app.hpp"


class GuiApp : public App {
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
