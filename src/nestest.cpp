#include <print>
#include <string>
#include <string_view>
#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "cart.hpp"
#include "cpu.hpp"
#include "decoder.hpp"


static std::string LogLine(u16 pc, const Instruction& instr, const Registers& regs) {
  u8 lo = static_cast<u8>(instr.arg);
  u8 hi = static_cast<u8>(instr.arg >> 8);

  std::string bytes_str;
  switch (instr.num_bytes) {
    case 1:
      bytes_str = std::format("{:02X}", instr.code);
      break;
    case 2:
      bytes_str = std::format("{:02X} {:02X}", instr.code, lo);
      break;
    case 3:
      bytes_str = std::format("{:02X} {:02X} {:02X}", instr.code, lo, hi);
      break;
    default: bytes_str = "hello world";
  }

  std::string instr_str = std::format("{:3}", magic_enum::enum_name(instr.op));

  u16 ppu_x = 1;
  u16 ppu_y = 1;
  size_t cyc = 1;

  std::string regs_str = std::format("A:{:02X} X:{:02X} Y:{:02X} P:{:02X} SP:{:02X} PPU:{:3},{:3} CYC:{}", regs.a, regs.x, regs.y, regs.p.val, regs.sp, ppu_x, ppu_y, cyc);

  return std::format("{:04X}  {:8}  {:30}  {}", pc, bytes_str, instr_str, regs_str);
}


static bool SetLoggingLevel(const std::string &level_name) {
  auto level = magic_enum::enum_cast<spdlog::level::level_enum>(level_name);
  if (level.has_value()) {
    spdlog::set_level(level.value());
    return true;
  }
  return false;
}

auto main(int argc, char *argv[]) -> int {
  spdlog::set_level(spdlog::level::trace);

  argparse::ArgumentParser program("acenes", "0.0.1");

  program.add_argument("--log-level")
    .help("Set the verbosity for logging")
    .default_value(std::string("trace"))
    .nargs(1);

  program.add_argument("rom")
    .default_value("./roms/nestest.nes")
    .help("path to nestest.nes ROM file");

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

  const std::string rom_path = program.get("rom");

  spdlog::info("Loading ROM: {}", rom_path);

  Cart cart;
  if (!cart.Load(rom_path)) {
    spdlog::error("Failed to load ROM!");
    return 1;
  }

  spdlog::info("ROM Loaded!");

  const auto& header = cart.Header();

  spdlog::info("Ident: {}", std::string(header.ident.begin(), header.ident.end()));
  spdlog::info("Console Type: {}", magic_enum::enum_name(header.GetConsoleType()));
  spdlog::info("Video Format: {}", magic_enum::enum_name(header.GetVideoFormat()));
  spdlog::info("Has Battery: {}", header.HasBattery());
  spdlog::info("Has Trainer: {}", header.HasTrainer());

  auto& rom = cart.Rom();

  StaticBuffer<0xFFFF> memory;
  std::copy(rom.begin(), rom.begin() + 0x4000, memory.begin() + 0x8000);
  std::copy(rom.begin(), rom.begin() + 0x4000, memory.begin() + 0xC000);

  Cpu cpu;
  cpu.registers.pc = 0xC000;
  cpu.memory = memory;

  while (true) {
    Registers before = cpu.registers;

    auto instr = Decoder::Decode(cpu.memory.data() + cpu.registers.pc);

    std::println("{}", LogLine(cpu.registers.pc, instr, cpu.registers));

    cpu.Step();

    Registers after = cpu.registers;
  }

  spdlog::info("Exiting.");

  return 0;
}
