#pragma once

#include <memory>

#include "app_options.hpp"


class App {
public:
  virtual ~App() = default;
  virtual int Run() = 0;

  static std::unique_ptr<App> CreateApplication(const AppOptions& options);
};
