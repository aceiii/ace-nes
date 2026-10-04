#include <filesystem>
#include <tuple>
#include <vector>
#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <simdjson.h>

#include "file.hpp"
#include "emulator.hpp"
#include "types.hpp"

namespace fs = std::filesystem;


auto SetLoggingLevel(std::string_view level_name) {
  auto level = magic_enum::enum_cast<spdlog::level::level_enum>(level_name);
  if (level.has_value()) {
    spdlog::set_level(level.value());
    return true;
  }
  return false;
}

struct AddrValue {
  u16 addr;
  u8 val;
};

struct RegAndMem {
  u16 pc;
  u8 s;
  u8 a;
  u8 x;
  u8 y;
  u8 p;
  std::vector<AddrValue> ram;
};

struct Cycle {
  enum Method {
    Read,
    Write,
  };

  u16 addr;
  u8 val;
  Method method;
};

struct SingleStepTest {
  std::string name;
  RegAndMem initial;
  RegAndMem final;
  std::vector<Cycle> cycles;
};

auto ParseRam(simdjson::dom::array arr) {
  std::vector<AddrValue> ram;
  ram.reserve(128);

  for (auto item : arr) {
    auto sub_array = item.get_array();
    u16 addr = sub_array.at(0).get_uint64();
    u8 val = sub_array.at(1).get_uint64();
    ram.emplace_back(addr, val);
  }

  return ram;
}

auto ParseRegAndMem(simdjson::dom::object obj) {
  return RegAndMem{
    .pc = static_cast<u16>(obj["pc"].get_uint64().value()),
    .s = static_cast<u8>(obj["s"].get_uint64().value()),
    .a = static_cast<u8>(obj["a"].get_uint64().value()),
    .x = static_cast<u8>(obj["x"].get_uint64().value()),
    .y = static_cast<u8>(obj["y"].get_uint64().value()),
    .p = static_cast<u8>(obj["p"].get_uint64().value()),
    .ram = ParseRam(obj["ram"].get_array()),
  };
}

auto ParseCycles(simdjson::dom::array arr) {
  std::vector<Cycle> cycles;
  cycles.reserve(128);

  for (auto item : arr) {
    auto sub_array = item.get_array();
    u16 addr = sub_array.at(0).get_uint64();
    u8 val = sub_array.at(1).get_uint64();
    std::string_view method_name = sub_array.at(2).get_string();

    auto method = method_name == "read" ? Cycle::Method::Read : Cycle::Method::Write;

    cycles.emplace_back(addr, val, method);
  }

  return cycles;
}

auto ParseSingleStepTest(simdjson::dom::object doc) {
  return SingleStepTest{
    .name = std::string(doc["name"].get_string().value()),
    .initial = ParseRegAndMem(doc["initial"].get_object().value()),
    .final = ParseRegAndMem(doc["final"].get_object().value()),
    .cycles = ParseCycles(doc["cycles"].get_array()),
  };
};


auto RunSingleStepTests(fs::path test_path) {
  simdjson::dom::parser parser;
  auto doc = parser.load(test_path.string());
  for (auto item : doc) {
    auto test = ParseSingleStepTest(item.get_object());

    spdlog::trace("name='{}'", test.name);
  }
}

auto main(int argc, char *argv[]) -> int {
  spdlog::set_level(spdlog::level::trace);

  argparse::ArgumentParser program("single-step-tests", "0.0.1");

  program.add_argument("--log-level")
    .help("Set the verbosity for logging")
    .default_value(std::string("trace"))
    .nargs(1);

  program.add_argument("test")
    .help("path to single step test file or directory")
    .nargs(1);

  try {
    program.parse_args(argc, argv);
  } catch (const std::exception &err) {
    std::cerr << err.what() << std::endl;
    std::cerr << program;
    return 1;
  }

  const std::string level = program.get("--log-level");
  if (!SetLoggingLevel(level)) {
    std::cerr << fmt::format("Invalid argument \"{}\" - allowed options: "
                             "{{trace, debug, info, warn, err, critical, off}}",
                             level)
              << std::endl;
    std::cerr << program;
    return 1;
  }

  auto test_path = fs::path(program.get("test"));
  if (!fs::exists(test_path)) {
    spdlog::error("Invalid test path: '{}'", test_path.string());
    return 1;
  }

  RunSingleStepTests(test_path);

  spdlog::info("Exiting.");

  return 0;
}
