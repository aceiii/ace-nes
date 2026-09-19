#include <print>
#include <string>
#include <string_view>
#include <argparse/argparse.hpp>
#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "bus.hpp"
#include "cart.hpp"
#include "cpu.hpp"
#include "decoder.hpp"


class TestBus : public IBus {
public:
  StaticBuffer<0x0800> ram {};
  StaticBuffer<0x2000> cart_ram {};
  StaticBuffer<0x4000> mem {};
  StaticBuffer<0x8> ppu {};
  Cart* cart;

  u8 Read(u16 address, BusMode mode = BusMode::Normal) override {
    u8 val = ([&]() -> u8 {
      if (address < 0x2000) {
        return ram[address % ram.size()];
      }

      if (address < 0x4000) {
        return ppu[address % ppu.size()];
      }

      if (address < 0x4018) {
        return 0xFF;
      }

      if (address < 0x8000) {
        return mem[address % 0x4000];
      }

      if (address >= 0x8000) {
        const auto& rom = cart->Rom();
        return rom[address % 0x4000];
      }

      return 0x00;
    })();

    spdlog::trace("TestBus::Read(0x{:04X}) => 0x{:02X}", address, val);
    return val;
  }

  void Write(u16 address, u8 value, BusMode mode = BusMode::Normal) override {
    spdlog::trace("TestBus::Write(0x{:04X}, 0x{:02x})", address, value);

    if (address < 0x2000) {
      ram[address % 0x0800] = value;
    }

    if (address < 0x4000) {
      ppu[address % 0x8] = value;
    }

    if (address < 0x4018) {
      return;
    }

    if (address < 0x8000) {
      mem[address % 0x4000] = value;
    }
  }

};

static inline auto Red(std::string_view sv) {
  return std::format("\033[1;31m{}\033[0m\n", sv);
}

static std::string HighlightMismatch(std::string_view input, std::string_view output) {
  int idx = -1;
  int n = std::max(input.size(), output.size());
  for (int i = 0; i < n; i++) {
    if (i >= input.size()) {
      idx = i;
      break;
    }

    auto c1 = input[i];
    auto c2 = output[i];
    if (c1 != c2) {
      idx = i;
      break;
    }
  }

  if (idx == -1) {
    return std::string(output);
  }
  if (idx == 0) {
    return Red(output);
  }

  return std::string(output.substr(0, idx)) + Red(output.substr(idx));
}

static std::string LogLine(const Instruction& instr, const Registers& regs, TestBus& bus, u64 cyc) {
  u16 pc = regs.pc;
  u8 lo = instr.lo;
  u8 hi = instr.hi;
  u16 arg = lo | (hi << 8);

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
  }

  bool abs_include_assignment;
  switch (instr.op) {
    case Op::LDA:
    case Op::LDX:
    case Op::STX:
    case Op::STA:
      abs_include_assignment = true;
      break;
    default:
      abs_include_assignment = false;
      break;
  }

  std::string instr_str = std::format("{:3}", magic_enum::enum_name(instr.op));
  switch (instr.addressing_mode) {
    case AddressingMode::Implicit: break;
    case AddressingMode::Accumulator:
      instr_str += " A";
      break;
    case AddressingMode::Immediate:
      instr_str += std::format(" #${:02X}", lo);
      break;
    case AddressingMode::ZeroPage:
      instr_str += std::format(" ${:02X} = {:02X}", lo, bus.Read(lo));
      break;
    case AddressingMode::Absolute:
      instr_str += std::format(" ${:04X}", arg);
      if (abs_include_assignment) {
        instr_str += std::format(" = {:02X}", bus.Read(arg));
      }
      break;
    case AddressingMode::Relative:
      instr_str += std::format(" ${:04X}", pc + instr.num_bytes + static_cast<i8>(lo));
      break;
    case AddressingMode::Indirect:
      instr_str += std::format(" (${:04X}) = {:04x}", arg, pc);
      break;
    case AddressingMode::IndexedZeroPageX:
      instr_str += std::format(" (${:02X},X) @ {:02X} = {:02X}", lo, lo + regs.x, bus.Read(lo + regs.x));
      break;
    case AddressingMode::IndexedZeroPageY:
      instr_str += std::format(" (${:02X}),Y @ {:02X} = {:02X}", lo, lo + regs.y, bus.Read(lo + regs.y));
      break;
    case AddressingMode::IndexedAbsoluteX:
    {
      auto addr = static_cast<u16>(arg + regs.x);
      instr_str += std::format(" ${:04X},X @ {:04X} = {:02X}", arg, addr, bus.Read(addr));
      break;
    }
    case AddressingMode::IndexedAbsoluteY:
    {
      auto addr = static_cast<u16>(arg + regs.y);
      instr_str += std::format(" ${:04X},Y @ {:04X} = {:02X}", arg, addr, bus.Read(addr));
      break;
    }
    case AddressingMode::IndexedIndirectX:
    {
      auto addr = bus.Read((lo + regs.x) % 0xFF) | (bus.Read((lo + regs.x + 1) % 0xFF) << 8);
      instr_str += std::format(" (${:02X},X) @ {:02X} = {:04X} = {:02X}", lo, lo, addr, bus.Read(addr));
      break;
    }
    case AddressingMode::IndexedIndirectY:
    {
      auto addr = (bus.Read(lo) | (bus.Read((lo + 1) & 0xFF) << 8)) + regs.y;
      instr_str += std::format(" (${:02X}),Y @ {:04X} = {:04X} = {:02X}", lo, addr, addr, bus.Read(addr));
      break;
    }
  }

  u16 ppu_x = 1;
  u16 ppu_y = 1;

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

  program.add_argument("testlog")
    .default_value("./roms/nestest.log")
    .help("path to nestest.log LOG file");

  program.add_argument("-x", "--early-exit")
    .default_value(false)
    .implicit_value(true)
    .help("exit after first error");

  program.add_argument("-n", "--exit-at")
    .scan<'i', int>()
    .default_value(-1)
    .help("exit after N lines");

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
  const std::string log_path = program.get("testlog");

  spdlog::info("Loading LOG: {}", log_path);

  auto log_lines = file::ReadLines(log_path);
  if (!log_lines.has_value()) {
    spdlog::error("Failed to load LOG!");
    return 1;
  }

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

  TestBus bus;
  bus.cart = &cart;

  Cpu cpu;
  cpu.cycles = 7;
  cpu.registers.a = 0;
  cpu.registers.x = 0;
  cpu.registers.y = 0;
  cpu.registers.pc = 0xC000;
  cpu.registers.sp = 0xfd;
  cpu.registers.p.val = 0x24;
  cpu.bus = &bus;

  const auto& lines = log_lines.value();
  u64 line_no = 0;

  auto early_exit = program.get<bool>("--early-exit");
  auto exit_at = program.get<int>("--exit-at");

  while (line_no < lines.size()) {
    u16 pc = cpu.registers.pc;

    auto byte = bus.Read(pc, BusMode::Direct);
    auto instr = Decoder::Decode(byte, pc);
    if (instr.num_bytes >= 2) {
      instr.lo = bus.Read(pc + 1, BusMode::Direct);
    }
    if (instr.num_bytes >= 3) {
      instr.hi = bus.Read(pc + 2, BusMode::Direct);
    }

    std::string line_out = std::format("{}", LogLine(instr, cpu.registers, bus, cpu.cycles)).substr(0, 73);

    cpu.Step();

    const auto& line_in = lines[line_no].substr(0, 73);

    spdlog::info("in  #{:<5} : {}", line_no, line_in);
    spdlog::info("out #{:<5} : {}", line_no, HighlightMismatch(lines[line_no], line_out));

    line_no += 1;

    if ((early_exit && line_in != line_out) || (exit_at && exit_at == line_no)) {
      break;
    }
  }

  spdlog::info("Exiting.");

  return 0;
}
