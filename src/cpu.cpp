#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "cpu.hpp"
#include "decoder.hpp"


namespace exec {
  u16 JMP(const Instruction& instr, const Registers& regs) {
    switch (instr.addressing_mode) {
      case AddressingMode::Absolute: return instr.arg;
      default: throw new std::logic_error("Addressing mode not implemented");
    }
  }

  void LDX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: regs.x = mem[instr.arg]; break;
      default: throw new std::logic_error("Addressing mode not implemented");
    }
  }
}

void Cpu::Step() {
  const auto& pc = registers.pc;

  auto instr = Decoder::Decode(memory.data() + pc);

  spdlog::debug("[0x{:04X}] 0x{:02X} ({})", pc, instr.code, magic_enum::enum_name(instr.op));

  u16 new_pc = registers.pc + instr.num_bytes;

  switch (instr.op) {
    case Op::JMP: new_pc = exec::JMP(instr, registers); break;
    case Op::LDX: exec::LDX(instr, registers, memory); break;
    default: throw new std::logic_error("Not implemented");
  }

  registers.pc = new_pc;

}
