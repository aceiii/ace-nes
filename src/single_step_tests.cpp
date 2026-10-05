#include <array>
#include <filesystem>
#include <format>
#include <tuple>
#include <vector>
#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>
#include <simdjson.h>

#include "cpu.hpp"
#include "emulator.hpp"
#include "file.hpp"
#include "string.hpp"
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

struct TestBus : public IBus {
  std::array<u8, 65536> mem;
  std::vector<Cycle> cycles;

  u8 Read(u16 address, BusMode mode = BusMode::Normal) override {
    spdlog::trace("TestBus::Read addr={:04X}, val={:02X}", address, mem[address]);
    if (mode == BusMode::Normal) {
      cycles.emplace_back(address, mem[address], Cycle::Method::Read);
    }
    return mem[address];
  }

  void Write(u16 address, u8 value, BusMode mode = BusMode::Normal) override {
    spdlog::trace("TestBus::Write addr={:04X}, val={:02X}", address, value);
    if (mode == BusMode::Normal) {
      cycles.emplace_back(address, value, Cycle::Method::Write);
    }
    mem[address] = value;
  }
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

auto SetRegistersAndMemory(Cpu& cpu, RegAndMem regmem) {
  cpu.registers.pc = regmem.pc;
  cpu.registers.sp = regmem.s;
  cpu.registers.a = regmem.a;
  cpu.registers.x = regmem.x;
  cpu.registers.y = regmem.y;
  cpu.registers.p.val = regmem.p;

  for (const auto& item : regmem.ram) {
    cpu.bus->Write(item.addr, item.val, BusMode::Direct);
  }
}

auto FormatRegisters(Cpu& cpu) {
  const auto& regs = cpu.registers;
  return std::format("pc={:04X}, a={:02X}, x={:02X}, y={:02X}, s={:02X}, p={:02X}", regs.pc, regs.a, regs.x, regs.y, regs.sp, regs.p.val);
}


struct MismatchedValue {
  std::string name;
  u16 expected;
  u16 actual;
};

template <typename T>
auto CompareAndPushMismatch(std::vector<MismatchedValue>& mismatches, std::string_view name, T expected, T actual) {
  if (expected != actual) {
    mismatches.emplace_back(std::string(name), expected, actual);
  }
}

auto CompareFinalRegisters(Cpu& cpu, RegAndMem regmem) {
  std::vector<MismatchedValue> results;
  results.reserve(128);

  const auto &regs = cpu.registers;

  CompareAndPushMismatch(results, "pc", regmem.pc, regs.pc);
  CompareAndPushMismatch(results, "a", regmem.a, regs.a);
  CompareAndPushMismatch(results, "x", regmem.x, regs.x);
  CompareAndPushMismatch(results, "y", regmem.y, regs.y);
  CompareAndPushMismatch(results, "s", regmem.s, regs.sp);
  CompareAndPushMismatch(results, "p", regmem.p, regs.p.val);

  for (const auto& mem : regmem.ram) {
    CompareAndPushMismatch(results, std::format("mem[{:02X}]", mem.addr), mem.val, cpu.bus->Read(mem.addr, BusMode::Direct));
  }

  return results;
}

auto FormatMismatches(const std::vector<MismatchedValue>& mismatches) {
  std::vector<std::string> results;
  results.reserve(128);

  for (const auto& item : mismatches) {
    results.push_back(std::format("{}:{:04X}!={:04X}", item.name, item.expected, item.actual));
  }

  return string::Join(results, ", ");
}

auto ParseSingleStepTestsJson(fs::path path) {
  std::vector<SingleStepTest> results;
  results.reserve(1024);

  simdjson::dom::parser parser;
  auto doc = parser.load(path.string());
  for (auto item : doc) {
    auto test = ParseSingleStepTest(item.get_object());
    results.push_back(test);
  }

  return results;
}

struct SingleStepTestConfig {
  fs::path test_path;
};

auto RunSingleStepTests(SingleStepTestConfig config) {
  auto tests = ParseSingleStepTestsJson(config.test_path);

  auto total = tests.size();
  int passed = 0;
  int failed = 0;

  auto test_index = 0;
  for (auto test : tests) {
    spdlog::info("Running test#{:04} name='{}'", test_index, test.name);

    Cpu cpu{};
    cpu.bus = std::make_shared<TestBus>();
    SetRegistersAndMemory(cpu, test.initial);

    spdlog::trace("before: {}", FormatRegisters(cpu));
    spdlog::trace("==================================");
    cpu.Step();
    spdlog::trace("==================================");
    spdlog::trace("after:  {}", FormatRegisters(cpu));

    auto result = CompareFinalRegisters(cpu, test.final);
    if (!result.empty()) {
      failed += 1;
      spdlog::error("Failed test#{:04} name='{}'", test_index, test.name);
      spdlog::error("Mismatches: {}", FormatMismatches(result));
    } else {
      passed += 1;
      spdlog::info("Passed test#{:04}", test_index);
    }

    test_index += 1;
  }

  spdlog::info("Test summary: {} total, {} passes, {} failed", total, passed, failed);
}


auto RunAllSingleStepTests(SingleStepTestConfig config) {
  std::vector<fs::path> test_files;
  test_files.reserve(128);

  for (const auto& entry : fs::directory_iterator(config.test_path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".json") {
      test_files.emplace_back(entry.path());
    }
  }

  spdlog::info("Found {} test files.", test_files.size());

  for (const auto& path : test_files) {
    auto new_config = config;
    new_config.test_path = path;

    RunSingleStepTests(new_config);
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

  if (fs::is_directory(test_path)) {
    RunAllSingleStepTests({
      .test_path = test_path,
    });
  } else {
    RunSingleStepTests({
      .test_path = test_path,
    });
  }

  spdlog::info("Exiting.");

  return 0;
}
