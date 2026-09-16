#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "cpu.hpp"
#include "decoder.hpp"


namespace exec {
  void Push(Registers& regs, std::span<u8> mem, u16 val) {
    u8 lo = val & 0xFF;
    u8 hi = val >> 8;

    mem[regs.sp--] = lo;
    mem[regs.sp--] = hi;
  }


  u16 JMP(const Instruction& instr, const Registers& regs) {
    switch (instr.addressing_mode) {
      case AddressingMode::Absolute: return instr.arg;
      case AddressingMode::Indirect: return instr.arg;
      default: std::unreachable();
    }
  }

  u16 JSR(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Absolute);
    Push(regs, mem, regs.pc + 2);
    return instr.arg;
  }

  u16 BCS(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    if (regs.p.carry) {
      return regs.pc + instr.num_bytes + instr.Offset();
    }
    return regs.pc + instr.num_bytes;
  }

  u16 BCC(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    if (!regs.p.carry) {
      return regs.pc + instr.num_bytes + instr.Offset();
    }
    return regs.pc + instr.num_bytes;
  }

  u16 BEQ(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    if (regs.p.zero) {
      return regs.pc + instr.num_bytes + instr.Offset();
    }
    return regs.pc + instr.num_bytes;
  }

  void LDA(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: regs.a = mem[instr.Lo()]; break;
      case AddressingMode::ZeroPage: regs.a = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: regs.a = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: regs.a = mem[instr.arg]; break;
      case AddressingMode::IndexedAbsoluteX: regs.a = mem[instr.arg + regs.x]; break;
      case AddressingMode::IndexedAbsoluteY: regs.a = mem[instr.arg + regs.y]; break;
      case AddressingMode::IndexedIndirectX: regs.a = mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)]; break;
      case AddressingMode::IndexedIndirectY: regs.a = mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y]; break;
      default: std::unreachable();
    }
  }

  void LDX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: regs.x = mem[instr.Lo()]; break;
      default: std::unreachable();
    }
  }

  void STX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: mem[instr.Lo()] = regs.y; break;
      case AddressingMode::IndexedZeroPageX: mem[(instr.Lo() + regs.y) % 0xFF] = regs.x; break;
      case AddressingMode::Absolute: mem[instr.arg] = regs.x; break;
      default: std::unreachable();
    }
  }

  void SEC(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.p.carry = 1;
  }

  void SED(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.p.decimal = 1;
  }

  void SEI(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.p.interrupt_disable = 1;
  }

  void CLC(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.p.carry = 0;
  }

  void CLD(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.p.decimal = 0;
  }
  void CLI(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.p.interrupt_disable = 0;
  }

  void CLV(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.p.overflow = 0;
  }
}

void Cpu::Step() {
  const auto& pc = registers.pc;

  auto instr = Decoder::Decode(memory.data() + pc);

  spdlog::debug("[0x{:04X}] 0x{:02X} ({})", pc, instr.code, magic_enum::enum_name(instr.op));

  u16 new_pc = registers.pc + instr.num_bytes;

  switch (instr.op) {
    case Op::NOP: break;
    case Op::JMP: new_pc = exec::JMP(instr, registers); break;
    case Op::JSR: new_pc = exec::JSR(instr, registers, memory); break;
    case Op::BCS: new_pc = exec::BCS(instr, registers); break;
    case Op::BCC: new_pc = exec::BCC(instr, registers); break;
    case Op::BEQ: new_pc = exec::BEQ(instr, registers); break;
    case Op::LDA: exec::LDA(instr, registers, memory); break;
    case Op::LDX: exec::LDX(instr, registers, memory); break;
    case Op::STX: exec::STX(instr, registers, memory); break;
    case Op::SEC: exec::SEC(instr, registers); break;
    case Op::SED: exec::SED(instr, registers); break;
    case Op::SEI: exec::SEI(instr, registers); break;
    case Op::CLC: exec::CLC(instr, registers); break;
    case Op::CLD: exec::CLD(instr, registers); break;
    case Op::CLI: exec::CLI(instr, registers); break;
    case Op::CLV: exec::CLV(instr, registers); break;
    default: throw new std::logic_error("Not implemented");
  }

  registers.pc = new_pc;

}
