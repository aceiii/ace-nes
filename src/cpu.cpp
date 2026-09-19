#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "bus.hpp"
#include "cpu.hpp"
#include "decoder.hpp"


namespace exec {
  constexpr const u16 kStackOffset = 0x0100;

  inline u8 ReadImmediate(Cpu* cpu) {
    return cpu->ReadNext();
  }

  inline u8 ReadZeroPage(Cpu* cpu) {
    return cpu->Read(cpu->ReadNext());
  }

  inline u16 ReadAbsolute(Cpu* cpu) {
    return cpu->Read(cpu->ReadNext16());
  }

  inline i8 ReadRelative(Cpu* cpu) {
    return static_cast<i8>(cpu->ReadNext());
  }

  inline u16 ReadIndirect(Cpu* cpu) {
    return cpu->registers.pc + cpu->ReadNext16();
  }

  inline u8 ReadIndexedZeroPageX(Cpu* cpu) {
    return cpu->Read((cpu->ReadNext() + cpu->registers.x) & 0xFF);
  }

  inline u8 ReadIndexedZeroPageY(Cpu* cpu) {
    return cpu->Read((cpu->ReadNext() + cpu->registers.y) & 0xFF);
  }

  inline u8 ReadIndexedAbsoluteX(Cpu* cpu) {
    return cpu->Read(cpu->ReadNext16() + cpu->registers.x);
  }

  inline u8 ReadIndexedAbsoluteY(Cpu* cpu) {
    return cpu->Read(cpu->ReadNext16() + cpu->registers.y);
  }

  inline u8 ReadIndexedIndirectX(Cpu* cpu) {
    return cpu->Read(cpu->Read((cpu->ReadNext() + cpu->registers.x) & 0xFF) | (cpu->Read((cpu->ReadNext() + cpu->registers.x + 1) & 0xFF) >> 8));
  }

  inline u8 ReadIndexedIndirectY(Cpu* cpu) {
    return cpu->Read((cpu->Read(cpu->ReadNext()) | (cpu->Read((cpu->ReadNext() + 1) & 0xFF) >> 8)) + cpu->registers.y);
  }

  inline void WriteZeroPage(Cpu* cpu, u8 val) {
    cpu->Write(cpu->ReadNext(), val);
  }

  inline void WriteAbsolute(Cpu* cpu, u8 val) {
    cpu->Write(cpu->ReadNext16(), val);
  }

  inline void WriteIndexedZeroPageX(Cpu* cpu, u8 val) {
    return cpu->Write((cpu->ReadNext() + cpu->registers.x) & 0xFF, val);
  }

  inline void WriteIndexedZeroPageY(Cpu* cpu, u8 val) {
    cpu->Write((cpu->ReadNext() + cpu->registers.y) & 0xFF, val);
  }

  inline void WriteIndexedAbsoluteX(Cpu* cpu, u8 val) {
    cpu->Write(cpu->ReadNext16() + cpu->registers.x, val);
  }

  inline void WriteIndexedAbsoluteY(Cpu* cpu, u8 val) {
    cpu->Write(cpu->ReadNext16() + cpu->registers.y, val);
  }

  inline void WriteIndexedIndirectX(Cpu* cpu, u8 val) {
    cpu->Write(cpu->Read((cpu->ReadNext() + cpu->registers.x) & 0xFF) | (cpu->Read((cpu->ReadNext() + cpu->registers.x + 1) & 0xFF) >> 8), val);
  }

  inline void WriteIndexedIndirectY(Cpu* cpu, u8 val) {
    cpu->Write((cpu->Read(cpu->ReadNext()) | (cpu->Read((cpu->ReadNext() + 1) & 0xFF) >> 8)) + cpu->registers.y, val);
  }

  inline void Push8(Cpu* cpu, u8 val) {
    cpu->Write(kStackOffset + cpu->registers.sp--, val);
  }

  inline u8 Pop8(Cpu* cpu) {
    return cpu->Read(kStackOffset + (++cpu->registers.sp));
  }

  inline void Push16(Cpu* cpu, u16 val) {
    u8 lo = val & 0xFF;
    u8 hi = val >> 8;

    Push8(cpu, lo);
    Push8(cpu, hi);
  }

  inline u16 Pop16(Cpu* cpu) {
    u8 hi = Pop8(cpu);
    u8 lo = Pop8(cpu);
    return lo | (hi << 8);
  }


  u16 JMP(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Absolute: return cpu->ReadNext16();
      case AddressingMode::Indirect: return ReadIndirect(cpu);
      default: std::unreachable();
    }
  }

  u16 JSR(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Absolute);
    u16 addr = cpu->ReadNext16();
    Push16(cpu, cpu->registers.pc);
    return addr;
  }

  u16 RTS(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    u16 pc = Pop16(cpu);
    return pc;
  }

  u16 BCS(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadRelative(cpu);
    if (cpu->registers.p.carry) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BCC(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadRelative(cpu);
    if (!cpu->registers.p.carry) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BEQ(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadRelative(cpu);
    if (cpu->registers.p.zero) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BNE(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadRelative(cpu);
    if (!cpu->registers.p.zero) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BMI(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadRelative(cpu);
    if (cpu->registers.p.negative) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BVS(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadRelative(cpu);
    if (cpu->registers.p.overflow) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BVC(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadRelative(cpu);
    if (!cpu->registers.p.overflow) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BPL(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadRelative(cpu);
    if (!cpu->registers.p.negative) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 LDA(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: cpu->registers.a = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: cpu->registers.a = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: cpu->registers.a = ReadIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: cpu->registers.a = ReadAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: cpu->registers.a = ReadIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: cpu->registers.a = ReadIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: cpu->registers.a = ReadIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: cpu->registers.a = ReadIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 LDX(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: cpu->registers.x = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: cpu->registers.x = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageY: cpu->registers.x = ReadIndexedZeroPageX(cpu); break;;
      case AddressingMode::Absolute: cpu->registers.x = ReadAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteY: cpu->registers.x = ReadIndexedAbsoluteY(cpu); break;
      default: std::unreachable();
    }
    cpu->registers.p.zero = cpu->registers.x == 0;
    cpu->registers.p.negative = (cpu->registers.x >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 LDY(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: cpu->registers.y = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: cpu->registers.y = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: cpu->registers.y = ReadIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: cpu->registers.y = ReadAbsolute(cpu); break;;
      case AddressingMode::IndexedAbsoluteX: cpu->registers.y = ReadIndexedAbsoluteX(cpu); break;
      default: std::unreachable();
    }
    cpu->registers.p.zero = cpu->registers.y == 0;
    cpu->registers.p.negative = (cpu->registers.y >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 STA(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: WriteZeroPage(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedZeroPageX: WriteIndexedZeroPageX(cpu, cpu->registers.a); break;
      case AddressingMode::Absolute: WriteAbsolute(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedAbsoluteX: WriteIndexedAbsoluteX(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedAbsoluteY: WriteIndexedAbsoluteY(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedIndirectX: WriteIndexedIndirectX(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedIndirectY: WriteIndexedIndirectY(cpu, cpu->registers.a); break;
      default: std::unreachable();
    }
    return cpu->registers.pc;
  }

  u16 STX(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: WriteZeroPage(cpu, cpu->registers.x); break;
      case AddressingMode::IndexedZeroPageY: WriteIndexedZeroPageY(cpu, cpu->registers.x); break;
      case AddressingMode::Absolute: WriteAbsolute(cpu, cpu->registers.x); break;
      default: std::unreachable();
    }
    return cpu->registers.pc;
  }

  u16 STY(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: WriteZeroPage(cpu, cpu->registers.y); break;
      case AddressingMode::IndexedZeroPageX: WriteIndexedZeroPageX(cpu, cpu->registers.y); break;
      case AddressingMode::Absolute: WriteAbsolute(cpu, cpu->registers.y); break;
      default: std::unreachable();
    }
    return cpu->registers.pc;
  }

  u16 SEC(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.p.carry = 1;
    return cpu->registers.pc;
  }

  u16 SED(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.p.decimal = 1;
    return cpu->registers.pc;
  }

  u16 SEI(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.p.interrupt_disable = 1;
    return cpu->registers.pc;
  }

  u16 CLC(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.p.carry = 0;
    return cpu->registers.pc;
  }

  u16 CLD(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.p.decimal = 0;
    return cpu->registers.pc;
  }
  u16 CLI(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.p.interrupt_disable = 0;
    return cpu->registers.pc;
  }

  u16 CLV(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.p.overflow = 0;
    return cpu->registers.pc;
  }

  u16 BIT(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::Absolute: val = ReadAbsolute(cpu); break;
      default: std::unreachable();
    }

    u8 new_val = cpu->registers.a & val;
    cpu->registers.p.zero = new_val == 0;
    cpu->registers.p.overflow = (val >> 6) & 0b1;
    cpu->registers.p.negative = (val >> 7) & 0b1;

    spdlog::trace("BIT instr: A={:02x}, lo={:02X}, hi={:02X}, val={:02X}, new_val={:02X}, zero={}, overflow={}, negative={}", cpu->registers.a, instr.lo, instr.hi, val, new_val, (u8)cpu->registers.p.zero, (u8)cpu->registers.p.overflow, (u8)cpu->registers.p.negative);
    return cpu->registers.pc;
  }

  u16 PHP(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    u8 status = cpu->registers.p.val;
    status |= (1 << 4) | (1 << 5);
    Push8(cpu, status);
    return cpu->registers.pc;
  }

  u16 PLP(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    auto new_status = std::bit_cast<StatusReg>(Pop8(cpu));
    new_status.ignored = cpu->registers.p.ignored;
    new_status.b_flag = cpu->registers.p.b_flag;
    cpu->registers.p = new_status;
    return cpu->registers.pc;
  }

  u16 PLA(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.a = Pop8(cpu);
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 PHA(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    Push8(cpu, cpu->registers.a);
    return cpu->registers.pc;
  }

  u16 AND(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: val = ReadIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }

    cpu->registers.a &= val;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 ORA(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: ReadIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }

    cpu->registers.a |= val;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 EOR(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: ReadIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }

    cpu->registers.a ^= val;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 CMP(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: val = ReadIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }

    u8 new_val = cpu->registers.a - val;
    cpu->registers.p.carry = cpu->registers.a >= val;
    cpu->registers.p.zero = new_val == 0;
    cpu->registers.p.negative = (new_val >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 CPX(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::Absolute: val =ReadAbsolute(cpu); break;
      default: std::unreachable();
    }

    u8 new_val = cpu->registers.x - val;
    cpu->registers.p.carry = cpu->registers.x >= val;
    cpu->registers.p.zero = new_val == 0;
    cpu->registers.p.negative = (new_val >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 CPY(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::Absolute: val = ReadAbsolute(cpu); break;
      default: std::unreachable();
    }

    u8 new_val = cpu->registers.y - val;
    cpu->registers.p.carry = cpu->registers.y >= val;
    cpu->registers.p.zero = new_val == 0;
    cpu->registers.p.negative = (new_val >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 ADC(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: ReadIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }

    u8 orig_a = cpu->registers.a;
    u16 new_val = cpu->registers.a + val + cpu->registers.p.carry;
    cpu->registers.a = new_val & 0xFF;
    cpu->registers.p.carry = new_val > 0xFF;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.overflow = (((cpu->registers.a ^ orig_a) & (cpu->registers.a ^ val)) >> 7) & 0b1;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 SBC(const Instruction& instr, Cpu* cpu) {
    u8 val;
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: val = ReadImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: val = ReadIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }

    u8 orig_a = cpu->registers.a;
    i16 new_val = static_cast<i16>(cpu->registers.a - val - (~cpu->registers.p.carry & 0b1));
    cpu->registers.a = new_val & 0xFF;
    cpu->registers.p.carry = ~(new_val < 0x00) & 0b1;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.overflow = (((cpu->registers.a ^ orig_a) & (cpu->registers.a ^ ~val)) >> 7) & 0b1;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 INY(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.y += 1;
    cpu->registers.p.zero = cpu->registers.y == 0;
    cpu->registers.p.negative = (cpu->registers.y >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 INX(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.x += 1;
    cpu->registers.p.zero = cpu->registers.x == 0;
    cpu->registers.p.negative = (cpu->registers.x >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 DEY(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.y -= 1;
    cpu->registers.p.zero = cpu->registers.y == 0;
    cpu->registers.p.negative = (cpu->registers.y >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 DEX(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.x -= 1;
    cpu->registers.p.zero = cpu->registers.x == 0;
    cpu->registers.p.negative = (cpu->registers.x >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 TAY(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.y = cpu->registers.a;
    cpu->registers.p.zero = cpu->registers.y == 0;
    cpu->registers.p.negative = (cpu->registers.y >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 TAX(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.x = cpu->registers.a;
    cpu->registers.p.zero = cpu->registers.x == 0;
    cpu->registers.p.negative = (cpu->registers.x >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 TYA(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.a = cpu->registers.y;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 TXA(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.a = cpu->registers.x;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 TSX(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.x = cpu->registers.sp;
    cpu->registers.p.zero = (cpu->registers.x == 0) & 0b1;
    cpu->registers.p.negative = (cpu->registers.x >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 TXS(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->registers.sp = cpu->registers.x;
    return cpu->registers.pc;
  }

  u16 BRK(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    cpu->ReadNext();
    return cpu->registers.pc;
  }
}

void Cpu::Step() {
  auto instr = Decoder::Decode(ReadNext(), registers.pc);
  if (instr.num_bytes >= 2) {
    instr.lo = bus->Read(registers.pc, BusMode::Direct);
  }
  if (instr.num_bytes >= 3) {
    instr.hi = bus->Read(registers.pc + 1, BusMode::Direct);
  }

  u16 new_pc = registers.pc;
  switch (instr.op) {
    case Op::NOP: Tick(); break;
    case Op::JMP: new_pc = exec::JMP(instr, this); break;
    case Op::JSR: new_pc = exec::JSR(instr, this); break;
    case Op::RTS: new_pc = exec::RTS(instr, this); break;
    case Op::BCS: new_pc = exec::BCS(instr, this); break;
    case Op::BCC: new_pc = exec::BCC(instr, this); break;
    case Op::BEQ: new_pc = exec::BEQ(instr, this); break;
    case Op::BNE: new_pc = exec::BNE(instr, this); break;
    case Op::BMI: new_pc = exec::BMI(instr, this); break;
    case Op::BVS: new_pc = exec::BVS(instr, this); break;
    case Op::BVC: new_pc = exec::BVC(instr, this); break;
    case Op::BPL: new_pc = exec::BPL(instr, this); break;
    case Op::LDA: new_pc = exec::LDA(instr, this); break;
    case Op::LDX: new_pc = exec::LDX(instr, this); break;
    case Op::LDY: new_pc = exec::LDY(instr, this); break;
    case Op::STA: new_pc = exec::STA(instr, this); break;
    case Op::STX: new_pc = exec::STX(instr, this); break;
    case Op::STY: new_pc = exec::STY(instr, this); break;
    case Op::SEC: new_pc = exec::SEC(instr, this); break;
    case Op::SED: new_pc = exec::SED(instr, this); break;
    case Op::SEI: new_pc = exec::SEI(instr, this); break;
    case Op::CLC: new_pc = exec::CLC(instr, this); break;
    case Op::CLD: new_pc = exec::CLD(instr, this); break;
    case Op::CLI: new_pc = exec::CLI(instr, this); break;
    case Op::CLV: new_pc = exec::CLV(instr, this); break;
    case Op::BIT: new_pc = exec::BIT(instr, this); break;
    case Op::PHP: new_pc = exec::PHP(instr, this); break;
    case Op::PLP: new_pc = exec::PLP(instr, this); break;
    case Op::PLA: new_pc = exec::PLA(instr, this); break;
    case Op::PHA: new_pc = exec::PHA(instr, this); break;
    case Op::AND: new_pc = exec::AND(instr, this); break;
    case Op::ORA: new_pc = exec::ORA(instr, this); break;
    case Op::EOR: new_pc = exec::EOR(instr, this); break;
    case Op::CMP: new_pc = exec::CMP(instr, this); break;
    case Op::CPX: new_pc = exec::CPX(instr, this); break;
    case Op::CPY: new_pc = exec::CPY(instr, this); break;
    case Op::ADC: new_pc = exec::ADC(instr, this); break;
    case Op::SBC: new_pc = exec::SBC(instr, this); break;
    case Op::INY: new_pc = exec::INY(instr, this); break;
    case Op::INX: new_pc = exec::INX(instr, this); break;
    case Op::DEY: new_pc = exec::DEY(instr, this); break;
    case Op::DEX: new_pc = exec::DEX(instr, this); break;
    case Op::TAY: new_pc = exec::TAY(instr, this); break;
    case Op::TAX: new_pc = exec::TAX(instr, this); break;
    case Op::TYA: new_pc = exec::TYA(instr, this); break;
    case Op::TXA: new_pc = exec::TXA(instr, this); break;
    case Op::TSX: new_pc = exec::TSX(instr, this); break;
    case Op::TXS: new_pc = exec::TXS(instr, this); break;
    case Op::BRK: new_pc = exec::BRK(instr, this); break;
    default:
      spdlog::critical("Instruction not implemented: {}({:02X}) @ 0x{:02X}", magic_enum::enum_name(instr.op), instr.code, instr.addr);
      throw new std::logic_error("Not implemented");
  }

  registers.pc = new_pc;
}

u8 Cpu::ReadNext() {
  u8 value = bus->Read(registers.pc++);
  Tick();
  return value;
}

u16 Cpu::ReadNext16() {
  u8 lo = ReadNext();
  u8 hi = ReadNext();
  return lo | (hi << 8);
}

u8 Cpu::Read(u16 address) {
  u8 value = bus->Read(address);
  Tick();
  return value;
}

void Cpu::Write(u16 address, u8 value) {
  bus->Write(address, value);
  Tick();
}

void Cpu::Tick() {
  cycles++;
}
