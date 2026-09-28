#pragma once

#include "app.hpp"
#include "app_options.hpp"


class HeadlessApp final : public App {
public:
  HeadlessApp(const AppOptions& options);

  int Run() override;

private:
  AppOptions options_;
};
