#include <string>
#include <string_view>
#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "app.hpp"
#include "options_parser.hpp"


auto main(int argc, char *argv[]) -> int {
  spdlog::set_level(spdlog::level::info);

  auto options = OptionsParser::ParseOptions(std::span<char*>(argv, argc));
  if (!options.has_value()) {
    std::cerr << options.error() << std::endl;
    return 1;
  }

  auto app = App::CreateApplication(options.value());
  return app->Run();
}
