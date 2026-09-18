#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "cpu.hpp"
#include "decoder.hpp"


namespace exec {
  inline void Push8(Registers& regs, std::span<u8> mem, u8 val) {
    mem[0x0100 + regs.sp--] = val;
  }

  inline u8 Pop8(Registers& regs, std::span<u8> mem) {
    return mem[0x0100 + (++regs.sp)];
  }

  inline void Push16(Registers& regs, std::span<u8> mem, u16 val) {
    u8 lo = val & 0xFF;
    u8 hi = val >> 8;

    Push8(regs, mem, lo);
    Push8(regs, mem, hi);
  }

  inline u16 Pop16(Registers& regs, std::span<u8> mem) {
    u8 hi = Pop8(regs, mem);
    u8 lo = Pop8(regs, mem);
    return lo | (hi << 8);
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
    Push16(regs, mem, regs.pc + 2);
    return instr.arg;
  }

  u16 RTS(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    u16 pc = Pop16(regs, mem);
    return pc + 1;
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

  u16 BNE(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    if (!regs.p.zero) {
      return regs.pc + instr.num_bytes + instr.Offset();
    }
    return regs.pc + instr.num_bytes;
  }

  u16 BMI(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    if (regs.p.negative) {
      return regs.pc + instr.num_bytes + instr.Offset();
    }
    return regs.pc + instr.num_bytes;
  }

  u16 BVS(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    if (regs.p.overflow) {
      return regs.pc + instr.num_bytes + instr.Offset();
    }
    return regs.pc + instr.num_bytes;
  }

  u16 BVC(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    if (!regs.p.overflow) {
      return regs.pc + instr.num_bytes + instr.Offset();
    }
    return regs.pc + instr.num_bytes;
  }

  u16 BPL(const Instruction& instr, Registers& regs) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    if (!regs.p.negative) {
      return regs.pc + instr.num_bytes + instr.Offset();
    }
    return regs.pc + instr.num_bytes;
  }

  void LDA(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: regs.a = instr.Lo(); break;
      case AddressingMode::ZeroPage: regs.a = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: regs.a = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: regs.a = mem[instr.arg]; break;
      case AddressingMode::IndexedAbsoluteX: regs.a = mem[instr.arg + regs.x]; break;
      case AddressingMode::IndexedAbsoluteY: regs.a = mem[instr.arg + regs.y]; break;
      case AddressingMode::IndexedIndirectX: regs.a = mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)]; break;
      case AddressingMode::IndexedIndirectY: regs.a = mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y]; break;
      default: std::unreachable();
    }
    regs.p.zero = regs.a == 0;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void LDX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: regs.x = instr.Lo(); break;
      case AddressingMode::ZeroPage: regs.x = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageY: regs.x = mem[(instr.Lo() + regs.y) & 0xFF]; break;
      case AddressingMode::Absolute: regs.x = mem[instr.Lo()]; break;
      case AddressingMode::IndexedAbsoluteY: regs.x = mem[instr.arg + regs.y]; break;
      default: std::unreachable();
    }
    regs.p.zero = regs.x == 0;
    regs.p.negative = (regs.x >> 7) & 0b1;
  }

  void LDY(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: regs.y = instr.Lo(); break;
      case AddressingMode::ZeroPage: regs.y = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: regs.y = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: regs.y = mem[instr.Lo()]; break;
      case AddressingMode::IndexedAbsoluteX: regs.y = mem[instr.arg + regs.x]; break;
      default: std::unreachable();
    }
    regs.p.zero = regs.y == 0;
    regs.p.negative = (regs.y >> 7) & 0b1;
  }

  void STA(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: mem[instr.Lo()] = regs.a; break;
      case AddressingMode::IndexedZeroPageX: mem[(instr.Lo() + regs.x) % 0xFF] = regs.a; break;
      case AddressingMode::Absolute: mem[instr.arg] = regs.a; break;
      case AddressingMode::IndexedAbsoluteX: mem[instr.arg + regs.x] = regs.a; break;
      case AddressingMode::IndexedAbsoluteY: mem[instr.arg + regs.y] = regs.a; break;
      case AddressingMode::IndexedIndirectX: mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)] = regs.a; break;
      case AddressingMode::IndexedIndirectY: mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y] = regs.a; break;
      default: std::unreachable();
    }
  }

  void STX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: mem[instr.Lo()] = regs.x; break;
      case AddressingMode::IndexedZeroPageY: mem[(instr.Lo() + regs.y) % 0xFF] = regs.x; break;
      case AddressingMode::Absolute: mem[instr.arg] = regs.x; break;
      default: std::unreachable();
    }
  }

  void STY(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: mem[instr.Lo()] = regs.y; break;
      case AddressingMode::IndexedZeroPageX: mem[(instr.Lo() + regs.x) % 0xFF] = regs.y; break;
      case AddressingMode::Absolute: mem[instr.arg] = regs.y; break;
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

  void BIT(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::Absolute: val = mem[instr.arg]; break;
      default: std::unreachable();
    }

    u8 new_val = regs.a & val;
    regs.p.zero = new_val == 0;
    regs.p.overflow = (new_val >> 6) & 0b1;
    regs.p.negative = (new_val >> 7) & 0b1;

    spdlog::trace("BIT instr: A={:02x}, arg={:04X}, val={:02X}, new_val={:02X}, zero={}, overflow={}, negative={}", regs.a, instr.arg, val, new_val, (u8)regs.p.zero, (u8)regs.p.overflow, (u8)regs.p.negative);
  }

  void PHP(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);

    u8 status = regs.p.val;
    status |= (1 << 4) | (1 << 5);

    Push8(regs, mem, status);
  }

  void PLP(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    auto new_status = std::bit_cast<StatusReg>(Pop8(regs, mem));
    new_status.ignored = regs.p.ignored;
    new_status.b_flag = regs.p.b_flag;
    regs.p = new_status;
  }

  void PLA(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.a = Pop8(regs, mem);
    regs.p.zero = regs.a == 0;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void PHA(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    Push8(regs, mem, regs.a);
  }

  void AND(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = instr.Lo(); break;
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: val = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: val = mem[instr.arg]; break;
      case AddressingMode::IndexedAbsoluteX: val = mem[instr.arg + regs.x]; break;
      case AddressingMode::IndexedAbsoluteY: val = mem[instr.arg + regs.y]; break;
      case AddressingMode::IndexedIndirectX: val = mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)]; break;
      case AddressingMode::IndexedIndirectY: val = mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y]; break;
      default: std::unreachable();
    }

    regs.a &= val;
    regs.p.zero = regs.a == 0;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void ORA(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = instr.Lo(); break;
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: val = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: val = mem[instr.arg]; break;
      case AddressingMode::IndexedAbsoluteX: val = mem[instr.arg + regs.x]; break;
      case AddressingMode::IndexedAbsoluteY: val = mem[instr.arg + regs.y]; break;
      case AddressingMode::IndexedIndirectX: val = mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)]; break;
      case AddressingMode::IndexedIndirectY: val = mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y]; break;
      default: std::unreachable();
    }

    regs.a |= val;
    regs.p.zero = regs.a == 0;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void EOR(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = instr.Lo(); break;
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: val = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: val = mem[instr.arg]; break;
      case AddressingMode::IndexedAbsoluteX: val = mem[instr.arg + regs.x]; break;
      case AddressingMode::IndexedAbsoluteY: val = mem[instr.arg + regs.y]; break;
      case AddressingMode::IndexedIndirectX: val = mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)]; break;
      case AddressingMode::IndexedIndirectY: val = mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y]; break;
      default: std::unreachable();
    }

    regs.a ^= val;
    regs.p.zero = regs.a == 0;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void CMP(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = instr.Lo(); break;
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: val = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: val = mem[instr.arg]; break;
      case AddressingMode::IndexedAbsoluteX: val = mem[instr.arg + regs.x]; break;
      case AddressingMode::IndexedAbsoluteY: val = mem[instr.arg + regs.y]; break;
      case AddressingMode::IndexedIndirectX: val = mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)]; break;
      case AddressingMode::IndexedIndirectY: val = mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y]; break;
      default: std::unreachable();
    }

    u8 new_val = regs.a - val;
    regs.p.carry = regs.a >= val;
    regs.p.zero = new_val == 0;
    regs.p.negative = (new_val >> 7) & 0b1;
  }

  void CPX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = instr.Lo(); break;
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::Absolute: val = mem[instr.arg]; break;
      default: std::unreachable();
    }

    u8 new_val = regs.x - val;
    regs.p.carry = regs.x >= val;
    regs.p.zero = new_val == 0;
    regs.p.negative = (new_val >> 7) & 0b1;
  }

  void CPY(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = instr.Lo(); break;
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::Absolute: val =mem[instr.arg]; break;
      default: std::unreachable();
    }

    u8 new_val = regs.y - val;
    regs.p.carry = regs.y >= val;
    regs.p.zero = new_val == 0;
    regs.p.negative = (new_val >> 7) & 0b1;
  }

  void ADC(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = instr.Lo(); break;
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: val = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: val = mem[instr.arg]; break;
      case AddressingMode::IndexedAbsoluteX: val = mem[instr.arg + regs.x]; break;
      case AddressingMode::IndexedAbsoluteY: val = mem[instr.arg + regs.y]; break;
      case AddressingMode::IndexedIndirectX: val = mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)]; break;
      case AddressingMode::IndexedIndirectY: val = mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y]; break;
      default: std::unreachable();
    }

    u8 orig_a = regs.a;
    u16 new_val = regs.a + val + regs.p.carry;
    regs.a = new_val & 0xFF;
    regs.p.carry = new_val > 0xFF;
    regs.p.zero = regs.a == 0;
    regs.p.overflow = (regs.a ^ orig_a) & (regs.a ^ val) & 0x80;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void SBC(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = instr.Lo(); break;
      case AddressingMode::ZeroPage: val = mem[instr.Lo()]; break;
      case AddressingMode::IndexedZeroPageX: val = mem[(instr.Lo() + regs.x) & 0xFF]; break;
      case AddressingMode::Absolute: val = mem[instr.arg]; break;
      case AddressingMode::IndexedAbsoluteX: val = mem[instr.arg + regs.x]; break;
      case AddressingMode::IndexedAbsoluteY: val = mem[instr.arg + regs.y]; break;
      case AddressingMode::IndexedIndirectX: val = mem[mem[(instr.Lo() + regs.x) & 0xFF] | (mem[(instr.Lo() + regs.x + 1) & 0xFF] >> 8)]; break;
      case AddressingMode::IndexedIndirectY: val = mem[(mem[instr.Lo()] | (mem[(instr.Lo() + 1) & 0xFF] >> 8)) + regs.y]; break;
      default: std::unreachable();
    }

    u8 orig_a = regs.a;
    u16 new_val = regs.a - val - ~regs.p.carry;
    regs.a = new_val & 0xFF;
    regs.p.carry = ~(new_val < 0x00);
    regs.p.zero = regs.a == 0;
    regs.p.overflow = (regs.a ^ orig_a) & (regs.a ^ ~val) & 0x80;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void INY(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.y += 1;
    regs.p.zero = regs.y == 0;
    regs.p.negative = (regs.y >> 7) & 0b1;
  }

  void INX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.x += 1;
    regs.p.zero = regs.x == 0;
    regs.p.negative = (regs.x >> 7) & 0b1;
  }

  void DEY(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.y -= 1;
    regs.p.zero = regs.y == 0;
    regs.p.negative = (regs.y >> 7) & 0b1;
  }

  void DEX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.x -= 1;
    regs.p.zero = regs.x == 0;
    regs.p.negative = (regs.x >> 7) & 0b1;
  }

  void TAY(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.y = regs.a;
    regs.p.zero = regs.y == 0;
    regs.p.negative = (regs.y >> 7) & 0b1;
  }

  void TAX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.x = regs.a;
    regs.p.zero = regs.x == 0;
    regs.p.negative = (regs.x >> 7) & 0b1;
  }

  void TYA(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.a = regs.y;
    regs.p.zero = regs.a == 0;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void TXA(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.a = regs.x;
    regs.p.zero = regs.a == 0;
    regs.p.negative = (regs.a >> 7) & 0b1;
  }

  void TSX(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.x = regs.sp;
    regs.p.zero = regs.x == 0;
    regs.p.negative = (regs.x >> 7) & 0b1;
  }

  void TXS(const Instruction& instr, Registers& regs, std::span<u8> mem) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    regs.sp = regs.x;
    regs.p.zero = regs.sp == 0;
    regs.p.negative = (regs.sp >> 7) & 0b1;
  }

}

void Cpu::Step() {
  const auto& pc = registers.pc;

  auto instr = Decoder::Decode(memory.data() + pc);
  Tick();

  if (instr.num_bytes > 1) {
    Tick();
  }

  if (instr.num_bytes > 2) {
    Tick();
  }

  u16 new_pc = registers.pc + instr.num_bytes;

  switch (instr.op) {
    case Op::NOP: break;
    case Op::JMP: new_pc = exec::JMP(instr, registers); break;
    case Op::JSR: new_pc = exec::JSR(instr, registers, memory); break;
    case Op::RTS: new_pc = exec::RTS(instr, registers, memory); break;
    case Op::BCS: new_pc = exec::BCS(instr, registers); break;
    case Op::BCC: new_pc = exec::BCC(instr, registers); break;
    case Op::BEQ: new_pc = exec::BEQ(instr, registers); break;
    case Op::BNE: new_pc = exec::BNE(instr, registers); break;
    case Op::BMI: new_pc = exec::BMI(instr, registers); break;
    case Op::BVS: new_pc = exec::BVS(instr, registers); break;
    case Op::BVC: new_pc = exec::BVC(instr, registers); break;
    case Op::BPL: new_pc = exec::BPL(instr, registers); break;
    case Op::LDA: exec::LDA(instr, registers, memory); break;
    case Op::LDX: exec::LDX(instr, registers, memory); break;
    case Op::LDY: exec::LDY(instr, registers, memory); break;
    case Op::STA: exec::STA(instr, registers, memory); break;
    case Op::STX: exec::STX(instr, registers, memory); break;
    case Op::STY: exec::STY(instr, registers, memory); break;
    case Op::SEC: exec::SEC(instr, registers); break;
    case Op::SED: exec::SED(instr, registers); break;
    case Op::SEI: exec::SEI(instr, registers); break;
    case Op::CLC: exec::CLC(instr, registers); break;
    case Op::CLD: exec::CLD(instr, registers); break;
    case Op::CLI: exec::CLI(instr, registers); break;
    case Op::CLV: exec::CLV(instr, registers); break;
    case Op::BIT: exec::BIT(instr, registers, memory); break;
    case Op::PHP: exec::PHP(instr, registers, memory); break;
    case Op::PLP: exec::PLP(instr, registers, memory); break;
    case Op::PLA: exec::PLA(instr, registers, memory); break;
    case Op::PHA: exec::PHA(instr, registers, memory); break;
    case Op::AND: exec::AND(instr, registers, memory); break;
    case Op::ORA: exec::ORA(instr, registers, memory); break;
    case Op::EOR: exec::EOR(instr, registers, memory); break;
    case Op::CMP: exec::CMP(instr, registers, memory); break;
    case Op::CPX: exec::CPX(instr, registers, memory); break;
    case Op::CPY: exec::CPY(instr, registers, memory); break;
    case Op::ADC: exec::ADC(instr, registers, memory); break;
    case Op::SBC: exec::SBC(instr, registers, memory); break;
    case Op::INY: exec::INY(instr, registers, memory); break;
    case Op::INX: exec::INX(instr, registers, memory); break;
    case Op::DEY: exec::DEY(instr, registers, memory); break;
    case Op::DEX: exec::DEX(instr, registers, memory); break;
    case Op::TAY: exec::TAY(instr, registers, memory); break;
    case Op::TAX: exec::TAX(instr, registers, memory); break;
    case Op::TYA: exec::TYA(instr, registers, memory); break;
    case Op::TXA: exec::TXA(instr, registers, memory); break;
    case Op::TSX: exec::TSX(instr, registers, memory); break;
    case Op::TXS: exec::TXS(instr, registers, memory); break;
    default:
      spdlog::critical("Instruction not implemented: {}({:02X})", magic_enum::enum_name(instr.op), instr.code);
      throw new std::logic_error("Not implemented");
  }

  registers.pc = new_pc;
}

void Cpu::Tick() {
  cycles++;
}
