#include <magic_enum/magic_enum.hpp>
#include <spdlog/spdlog.h>

#include "address.hpp"
#include "bus.hpp"
#include "cpu.hpp"
#include "decoder.hpp"


namespace exec {
  constexpr const u16 kStackOffset = 0x0100;

  inline u16 ReadIndexedIndirectY(Cpu* cpu, u8 byte) {
    return (cpu->Read(byte) | (cpu->Read((byte + 1) & 0xFF) << 8)) + cpu->registers.y;
  }

  inline u16 ReadIndexedIndirectX(Cpu* cpu, u8 byte) {
    return cpu->Read((byte + cpu->registers.x) & 0xFF) | (cpu->Read((byte + cpu->registers.x + 1) & 0xFF) << 8);
  }

  inline u8 ReadNextImmediate(Cpu* cpu) {
    return cpu->ReadNext();
  }

  inline u8 ReadNextZeroPage(Cpu* cpu) {
    return cpu->Read(cpu->ReadNext());
  }

  inline u8 ReadNextAbsolute(Cpu* cpu) {
    return cpu->Read(cpu->ReadNext16());
  }

  inline i8 ReadNextRelative(Cpu* cpu) {
    return static_cast<i8>(cpu->ReadNext());
  }

  inline u8 ReadNextIndexedZeroPageX(Cpu* cpu) {
    return cpu->Read(address::IndexedZeroPageX(cpu, cpu->ReadNext()));
  }

  inline u8 ReadNextIndexedZeroPageY(Cpu* cpu) {
    return cpu->Read(address::IndexedZeroPageY(cpu, cpu->ReadNext()));
  }

  inline u8 ReadNextIndexedAbsoluteX(Cpu* cpu) {
    return cpu->Read(address::IndexedAbsoluteX(cpu, cpu->ReadNext16()));
  }

  inline u8 ReadNextIndexedAbsoluteY(Cpu* cpu) {
    return cpu->Read(address::IndexedAbsoluteY(cpu, cpu->ReadNext16()));
  }

  inline u8 ReadNextIndexedIndirectX(Cpu* cpu) {
    return cpu->Read(ReadIndexedIndirectX(cpu, cpu->ReadNext()));
  }

  inline u8 ReadNextIndexedIndirectY(Cpu* cpu) {
    return cpu->Read(ReadIndexedIndirectY(cpu, cpu->ReadNext()));
  }

  inline void WriteZeroPage(Cpu* cpu, u8 addr, u8 val) {
    cpu->Write(addr, val);
  }

  inline void WriteNextZeroPage(Cpu* cpu, u8 val) {
    WriteZeroPage(cpu, cpu->ReadNext(), val);
  }

  inline void WriteNextAbsolute(Cpu* cpu, u8 val) {
    cpu->Write(cpu->ReadNext16(), val);
  }

  inline void WriteIndexedZeroPageX(Cpu* cpu, u8 byte, u8 val) {
    return cpu->Write((byte + cpu->registers.x) & 0xFF, val);
  }

  inline void WriteNextIndexedZeroPageX(Cpu* cpu, u8 val) {
    return cpu->Write((cpu->ReadNext() + cpu->registers.x) & 0xFF, val);
  }

  inline void WriteNextIndexedZeroPageY(Cpu* cpu, u8 val) {
    cpu->Write((cpu->ReadNext() + cpu->registers.y) & 0xFF, val);
  }

  inline void WriteNextIndexedAbsoluteX(Cpu* cpu, u8 val) {
    cpu->Write(address::IndexedAbsoluteX(cpu, cpu->ReadNext16()), val);
  }

  inline void WriteNextIndexedAbsoluteY(Cpu* cpu, u8 val) {
    cpu->Write(cpu->ReadNext16() + cpu->registers.y, val);
  }

  inline void WriteNextIndexedIndirectX(Cpu* cpu, u8 val) {
    u8 addr = cpu->ReadNext();
    cpu->Write(cpu->Read((addr + cpu->registers.x) & 0xFF) | (cpu->Read((addr + cpu->registers.x + 1) & 0xFF) << 8), val);
  }

  inline void WriteNextIndexedIndirectY(Cpu* cpu, u8 val) {
    cpu->Write(ReadIndexedIndirectY(cpu, cpu->ReadNext()), val);
  }

  inline void Push8(Cpu* cpu, u8 val) {
    spdlog::trace("Push8  @{:02X} = {:02X}", cpu->registers.sp, val);
    cpu->Write(kStackOffset + cpu->registers.sp--, val);
  }

  inline u8 Pop8(Cpu* cpu) {
    u8 result = cpu->Read(kStackOffset + (++cpu->registers.sp));
    spdlog::trace("Pop8  @{:02X} = {:02X}", cpu->registers.sp, result);
    return result;
  }

  inline void Push16(Cpu* cpu, u16 val) {
    u8 lo = val & 0xFF;
    u8 hi = val >> 8;

    Push8(cpu, hi);
    Push8(cpu, lo);
  }

  inline u16 Pop16(Cpu* cpu) {
    u8 lo = Pop8(cpu);
    u8 hi = Pop8(cpu);
    return lo | (hi << 8);
  }

  u16 NOP(const Instruction& instr, Cpu* cpu) {
    spdlog::trace("NOP({:02X}) num_bytes={}", instr.code, instr.num_bytes);
    int i = instr.num_bytes - 1;
    while (i--) {
      cpu->ReadNext();
    }
    cpu->Tick();
    return cpu->registers.pc;
  }

  u16 JMP(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Absolute: return cpu->ReadNext16();
      case AddressingMode::Indirect: {
        u16 first = cpu->ReadNext16();
        u16 second = (first & 0xFF00) | ((first + 1) & 0xFF);
        u8 lo = cpu->Read(first);
        u8 hi = cpu->Read(second);
        return lo | (hi << 8);
      }
      default: std::unreachable();
    }
  }

  u16 JSR(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Absolute);
    u16 addr = cpu->ReadNext16();
    Push16(cpu, cpu->registers.pc-1);
    return addr;
  }

  u16 RTS(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);
    u16 pc = Pop16(cpu);
    return pc + 1;
  }

  u16 RTI(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Implicit);

    StatusReg stat = std::bit_cast<StatusReg>(Pop8(cpu));
    u16 addr = Pop16(cpu);

    cpu->registers.p.carry = stat.carry;
    cpu->registers.p.zero = stat.zero;
    cpu->registers.p.interrupt_disable = stat.interrupt_disable;
    cpu->registers.p.decimal = stat.decimal;
    cpu->registers.p.overflow = stat.overflow;
    cpu->registers.p.negative = stat.negative;
    return addr;
  }

  u16 BCS(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadNextRelative(cpu);
    if (cpu->registers.p.carry) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BCC(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadNextRelative(cpu);
    if (!cpu->registers.p.carry) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BEQ(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadNextRelative(cpu);
    if (cpu->registers.p.zero) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BNE(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadNextRelative(cpu);
    if (!cpu->registers.p.zero) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BMI(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadNextRelative(cpu);
    if (cpu->registers.p.negative) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BVS(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadNextRelative(cpu);
    if (cpu->registers.p.overflow) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BVC(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadNextRelative(cpu);
    if (!cpu->registers.p.overflow) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 BPL(const Instruction& instr, Cpu* cpu) {
    assert(instr.addressing_mode == AddressingMode::Relative);
    i8 offset = ReadNextRelative(cpu);
    if (!cpu->registers.p.negative) {
      return cpu->registers.pc + offset;
    }
    return cpu->registers.pc;
  }

  u16 LDA(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: cpu->registers.a = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: cpu->registers.a = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: cpu->registers.a = ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: cpu->registers.a = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: cpu->registers.a = ReadNextIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: cpu->registers.a = ReadNextIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: cpu->registers.a = ReadNextIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: cpu->registers.a = ReadNextIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 LDX(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: cpu->registers.x = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: cpu->registers.x = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageY: cpu->registers.x = ReadNextIndexedZeroPageY(cpu); break;
      case AddressingMode::Absolute: cpu->registers.x = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteY: cpu->registers.x = ReadNextIndexedAbsoluteY(cpu); break;
      default: std::unreachable();
    }
    cpu->registers.p.zero = cpu->registers.x == 0;
    cpu->registers.p.negative = (cpu->registers.x >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 LDY(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: cpu->registers.y = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: cpu->registers.y = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: cpu->registers.y = ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: cpu->registers.y = ReadNextAbsolute(cpu); break;;
      case AddressingMode::IndexedAbsoluteX: cpu->registers.y = ReadNextIndexedAbsoluteX(cpu); break;
      default: std::unreachable();
    }
    cpu->registers.p.zero = cpu->registers.y == 0;
    cpu->registers.p.negative = (cpu->registers.y >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 STA(const Instruction& instr, Cpu* cpu) {
    spdlog::trace("pc before: {:02X}", cpu->registers.pc);
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: WriteNextZeroPage(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedZeroPageX: WriteNextIndexedZeroPageX(cpu, cpu->registers.a); break;
      case AddressingMode::Absolute: WriteNextAbsolute(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedAbsoluteX: WriteNextIndexedAbsoluteX(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedAbsoluteY: WriteNextIndexedAbsoluteY(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedIndirectX: WriteNextIndexedIndirectX(cpu, cpu->registers.a); break;
      case AddressingMode::IndexedIndirectY: WriteNextIndexedIndirectY(cpu, cpu->registers.a); break;
      default: std::unreachable();
    }
    spdlog::trace("pc after: {:02X}", cpu->registers.pc);
    return cpu->registers.pc;
  }

  u16 STX(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: WriteNextZeroPage(cpu, cpu->registers.x); break;
      case AddressingMode::IndexedZeroPageY: WriteNextIndexedZeroPageY(cpu, cpu->registers.x); break;
      case AddressingMode::Absolute: WriteNextAbsolute(cpu, cpu->registers.x); break;
      default: std::unreachable();
    }
    return cpu->registers.pc;
  }

  u16 STY(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: WriteNextZeroPage(cpu, cpu->registers.y); break;
      case AddressingMode::IndexedZeroPageX: WriteNextIndexedZeroPageX(cpu, cpu->registers.y); break;
      case AddressingMode::Absolute: WriteNextAbsolute(cpu, cpu->registers.y); break;
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
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::Absolute: val = ReadNextAbsolute(cpu); break;
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
      case AddressingMode::Immediate: val = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: val = ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadNextIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadNextIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadNextIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadNextIndexedIndirectY(cpu); break;
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
      case AddressingMode::Immediate: val = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadNextIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadNextIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadNextIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadNextIndexedIndirectY(cpu); break;
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
      case AddressingMode::Immediate: val = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadNextIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadNextIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadNextIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadNextIndexedIndirectY(cpu); break;
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
      case AddressingMode::Immediate: val = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: val = ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadNextIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadNextIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadNextIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadNextIndexedIndirectY(cpu); break;
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
      case AddressingMode::Immediate: val = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::Absolute: val =ReadNextAbsolute(cpu); break;
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
      case AddressingMode::Immediate: val = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::Absolute: val = ReadNextAbsolute(cpu); break;
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
      case AddressingMode::Immediate: val = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadNextIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadNextIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadNextIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadNextIndexedIndirectY(cpu); break;
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
      case AddressingMode::Immediate: val = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: val = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: val = ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::Absolute: val = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: val = ReadNextIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: val = ReadNextIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: val = ReadNextIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: val = ReadNextIndexedIndirectY(cpu); break;
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
    cpu->Tick();
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
    cpu->registers.p.zero = cpu->registers.x == 0;
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

  u16 LSR(const Instruction& instr, Cpu* cpu) {
    u8 val, result;
    switch (instr.addressing_mode) {
      case AddressingMode::Accumulator: {
        val = cpu->registers.a;
        result = val >> 1;
        cpu->registers.a = result;
        cpu->Tick();
        break;
      }
      case AddressingMode::ZeroPage: {
        u8 addr = cpu->ReadNext();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        WriteZeroPage(cpu, addr, result);
        break;
      }
      case AddressingMode::IndexedZeroPageX: {
        u8 addr = address::IndexedZeroPageX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::Absolute: {
        u16 addr = cpu->ReadNext16();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteX: {
        u16 addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      default: std::unreachable();
    }

    cpu->registers.p.carry = val & 0b1;
    cpu->registers.p.zero = result == 0;
    cpu->registers.p.negative = 0;
    return cpu->registers.pc;
  }

  u16 ASL(const Instruction& instr, Cpu* cpu) {
    u8 val, result;
    switch (instr.addressing_mode) {
      case AddressingMode::Accumulator: {
        val = cpu->registers.a;
        result = val << 1;
        cpu->registers.a = result;
        cpu->Tick();
        break;
      }
      case AddressingMode::ZeroPage: {
        u8 addr = cpu->ReadNext();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        WriteZeroPage(cpu, addr, result);
        break;
      }
      case AddressingMode::IndexedZeroPageX: {
        u8 addr = address::IndexedZeroPageX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::Absolute: {
        u16 addr = cpu->ReadNext16();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteX: {
        u16 addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      default: std::unreachable();
    }

    cpu->registers.p.carry = (val >> 7) & 0b1;
    cpu->registers.p.zero = result == 0;
    cpu->registers.p.negative = (result >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 ROR(const Instruction& instr, Cpu* cpu) {
    u8 val, result;
    switch (instr.addressing_mode) {
      case AddressingMode::Accumulator: {
        val = cpu->registers.a;
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.a = result;
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        break;
      }
      case AddressingMode::ZeroPage: {
        u8 addr = cpu->ReadNext();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        WriteZeroPage(cpu, addr, result);
        break;
      }
      case AddressingMode::IndexedZeroPageX: {
        u8 addr = address::IndexedZeroPageX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::Absolute: {
        u16 addr = cpu->ReadNext16();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteX: {
        u16 addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      default: std::unreachable();
    }

    cpu->registers.p.carry = val & 0b1;
    cpu->registers.p.zero = result == 0;
    cpu->registers.p.negative = (result >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 ROL(const Instruction& instr, Cpu* cpu) {
    u8 val, result;
    switch (instr.addressing_mode) {
      case AddressingMode::Accumulator: {
        val = cpu->registers.a;
        u8 new_c = (val >> 7) & 0b1;
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.a = result;
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        break;
      }
      case AddressingMode::ZeroPage: {
        u8 addr = cpu->ReadNext();
        val = cpu->Read(addr);
        u8 new_c = (val >> 7) & 0b1;
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->Tick();
        WriteZeroPage(cpu, addr, result);
        break;
      }
      case AddressingMode::IndexedZeroPageX: {
        u8 addr = address::IndexedZeroPageX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = (val >> 7) & 0b1;
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::Absolute: {
        u16 addr = cpu->ReadNext16();
        val = cpu->Read(addr);
        u8 new_c = (val >> 7) & 0b1;
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteX: {
        u16 addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16());
        cpu->Tick();
        u8 new_c = (val >> 7) & 0b1;
        val = cpu->Read(addr);
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      default: std::unreachable();
    }

    cpu->registers.p.carry = (val >> 7) & 0b1;
    cpu->registers.p.zero = result == 0;
    cpu->registers.p.negative = (result >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 INC(const Instruction& instr, Cpu* cpu) {
    u16 addr;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: addr = cpu->ReadNext(); break;
      case AddressingMode::IndexedZeroPageX: addr = address::IndexedZeroPageX(cpu, cpu->ReadNext()); break;
      case AddressingMode::Absolute: addr = cpu->ReadNext16(); break;
      case AddressingMode::IndexedAbsoluteX: addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16()); break;
      default: std::unreachable();
    }

    u8 result = cpu->Increment(addr);
    cpu->registers.p.zero = result == 0;
    cpu->registers.p.negative = (result >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 DEC(const Instruction& instr, Cpu* cpu) {
    u16 addr;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: addr = cpu->ReadNext(); break;
      case AddressingMode::IndexedZeroPageX: addr = address::IndexedZeroPageX(cpu, cpu->ReadNext()); break;
      case AddressingMode::Absolute: addr = cpu->ReadNext16(); break;
      case AddressingMode::IndexedAbsoluteX: addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16()); break;
      default: std::unreachable();
    }

    u8 result = cpu->Decrement(addr);
    cpu->registers.p.zero = result == 0;
    cpu->registers.p.negative = (result >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 LAX(const Instruction& instr, Cpu* cpu) {
    switch (instr.addressing_mode) {
      case AddressingMode::Immediate: cpu->registers.a = ReadNextImmediate(cpu); break;
      case AddressingMode::ZeroPage: cpu->registers.a = ReadNextZeroPage(cpu); break;
      case AddressingMode::IndexedZeroPageX: cpu->registers.a = ReadNextIndexedZeroPageX(cpu); break;
      case AddressingMode::IndexedZeroPageY: cpu->registers.a = ReadNextIndexedZeroPageY(cpu); break;
      case AddressingMode::Absolute: cpu->registers.a = ReadNextAbsolute(cpu); break;
      case AddressingMode::IndexedAbsoluteX: cpu->registers.a = ReadNextIndexedAbsoluteX(cpu); break;
      case AddressingMode::IndexedAbsoluteY: cpu->registers.a = ReadNextIndexedAbsoluteY(cpu); break;
      case AddressingMode::IndexedIndirectX: cpu->registers.a = ReadNextIndexedIndirectX(cpu); break;
      case AddressingMode::IndexedIndirectY: cpu->registers.a = ReadNextIndexedIndirectY(cpu); break;
      default: std::unreachable();
    }

    cpu->registers.x = cpu->registers.a;
    cpu->registers.p.zero = cpu->registers.x == 0;
    cpu->registers.p.negative = (cpu->registers.x >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 SAX(const Instruction& instr, Cpu* cpu) {
    u8 val = cpu->registers.a & cpu->registers.x;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: WriteNextZeroPage(cpu, val); break;
      case AddressingMode::IndexedZeroPageX: WriteNextIndexedZeroPageX(cpu, val); break;
      case AddressingMode::IndexedZeroPageY: WriteNextIndexedZeroPageY(cpu, val); break;
      case AddressingMode::Absolute: WriteNextAbsolute(cpu, val); break;
      case AddressingMode::IndexedAbsoluteX: WriteNextIndexedAbsoluteX(cpu, val); break;
      case AddressingMode::IndexedAbsoluteY: WriteNextIndexedAbsoluteY(cpu, val); break;
      case AddressingMode::IndexedIndirectX: WriteNextIndexedIndirectX(cpu, val); break;
      case AddressingMode::IndexedIndirectY: WriteNextIndexedIndirectY(cpu, val); break;
      default: std::unreachable();
    }
    return cpu->registers.pc;
  }

  u16 DCP(const Instruction& instr, Cpu* cpu) {
    u16 addr;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: addr = cpu->ReadNext(); break;
      case AddressingMode::IndexedZeroPageX: addr = address::IndexedZeroPageX(cpu, cpu->ReadNext()); break;
      case AddressingMode::Absolute: addr = cpu->ReadNext16(); break;
      case AddressingMode::IndexedAbsoluteX: addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16()); break;
      case AddressingMode::IndexedAbsoluteY: addr = address::IndexedAbsoluteY(cpu, cpu->ReadNext16()); break;
      case AddressingMode::IndexedIndirectX: addr = ReadIndexedIndirectX(cpu, cpu->ReadNext()); break;
      case AddressingMode::IndexedIndirectY: addr = ReadIndexedIndirectY(cpu, cpu->ReadNext()); break;
      default: std::unreachable();
    }

    u8 result = cpu->Decrement(addr);

    u8 new_val = cpu->registers.a - result;
    cpu->registers.p.carry = cpu->registers.a >= result;
    cpu->registers.p.zero = new_val == 0;
    cpu->registers.p.negative = (new_val >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 ISB(const Instruction& instr, Cpu* cpu) {
    u16 addr;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: addr = cpu->ReadNext(); break;
      case AddressingMode::IndexedZeroPageX: addr = address::IndexedZeroPageX(cpu, cpu->ReadNext()); break;
      case AddressingMode::Absolute: addr = cpu->ReadNext16(); break;
      case AddressingMode::IndexedAbsoluteX: addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16()); break;
      case AddressingMode::IndexedAbsoluteY: addr = address::IndexedAbsoluteY(cpu, cpu->ReadNext16()); break;
      case AddressingMode::IndexedIndirectX: addr = ReadIndexedIndirectX(cpu, cpu->ReadNext()); break;
      case AddressingMode::IndexedIndirectY: addr = ReadIndexedIndirectY(cpu, cpu->ReadNext()); break;
      default: std::unreachable();
    }

    u8 result = cpu->Increment(addr);

    u8 orig_a = cpu->registers.a;
    i16 new_val = static_cast<i16>(cpu->registers.a - result - (~cpu->registers.p.carry & 0b1));
    cpu->registers.a = new_val & 0xFF;
    cpu->registers.p.carry = ~(new_val < 0x00) & 0b1;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.overflow = (((cpu->registers.a ^ orig_a) & (cpu->registers.a ^ ~result)) >> 7) & 0b1;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 SLO(const Instruction& instr, Cpu* cpu) {
    u8 val, result;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: {
        u8 addr = cpu->ReadNext();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        WriteZeroPage(cpu, addr, result);
        break;
      }
      case AddressingMode::IndexedZeroPageX: {
        u8 addr = address::IndexedZeroPageX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::Absolute: {
        u16 addr = cpu->ReadNext16();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteX: {
        u16 addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteY: {
        u16 addr = address::IndexedAbsoluteY(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedIndirectX: {
        u16 addr = ReadIndexedIndirectX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedIndirectY: {
        u16 addr = ReadIndexedIndirectY(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val << 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      default: std::unreachable();
    }

    cpu->registers.a |= result;
    cpu->registers.p.carry = (val >> 7) & 0b1;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;

    return cpu->registers.pc;
  }

  u16 SRE(const Instruction& instr, Cpu* cpu) {
    u8 val, result;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: {
        u8 addr = cpu->ReadNext();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        WriteZeroPage(cpu, addr, result);
        break;
      }
      case AddressingMode::IndexedZeroPageX: {
        u8 addr = address::IndexedZeroPageX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::Absolute: {
        u16 addr = cpu->ReadNext16();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteX: {
        u16 addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteY: {
        u16 addr = address::IndexedAbsoluteY(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedIndirectX: {
        u16 addr = ReadIndexedIndirectX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedIndirectY: {
        u16 addr = ReadIndexedIndirectY(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        result = val >> 1;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      default: std::unreachable();
    }

    cpu->registers.a ^= result;
    cpu->registers.p.carry = val & 0b1;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 RLA(const Instruction& instr, Cpu* cpu) {
    u8 val, result;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: {
        u8 addr = cpu->ReadNext();
        val = cpu->Read(addr);
        u8 new_c = (val >> 7) & 0b1;
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->Tick();
        WriteZeroPage(cpu, addr, result);
        break;
      }
      case AddressingMode::IndexedZeroPageX: {
        u8 addr = address::IndexedZeroPageX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = (val >> 7) & 0b1;
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::Absolute: {
        u16 addr = cpu->ReadNext16();
        val = cpu->Read(addr);
        u8 new_c = (val >> 7) & 0b1;
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteX: {
        u16 addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16());
        cpu->Tick();
        u8 new_c = (val >> 7) & 0b1;
        val = cpu->Read(addr);
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteY: {
        u16 addr = address::IndexedAbsoluteY(cpu, cpu->ReadNext16());
        cpu->Tick();
        u8 new_c = (val >> 7) & 0b1;
        val = cpu->Read(addr);
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedIndirectX: {
        u16 addr = ReadIndexedIndirectX(cpu, cpu->ReadNext());
        cpu->Tick();
        u8 new_c = (val >> 7) & 0b1;
        val = cpu->Read(addr);
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedIndirectY: {
        u16 addr = ReadIndexedIndirectY(cpu, cpu->ReadNext());
        cpu->Tick();
        u8 new_c = (val >> 7) & 0b1;
        val = cpu->Read(addr);
        result = ((val << 1) & 0xFE) | (cpu->registers.p.carry & 0b1);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      default: std::unreachable();
    }

    cpu->registers.a &= result;
    cpu->registers.p.carry = (val >> 7) & 0b1;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
    return cpu->registers.pc;
  }

  u16 RRA(const Instruction& instr, Cpu* cpu) {
    u8 val, result;
    switch (instr.addressing_mode) {
      case AddressingMode::ZeroPage: {
        u8 addr = cpu->ReadNext();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        WriteZeroPage(cpu, addr, result);
        break;
      }
      case AddressingMode::IndexedZeroPageX: {
        u8 addr = address::IndexedZeroPageX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::Absolute: {
        u16 addr = cpu->ReadNext16();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteX: {
        u16 addr = address::IndexedAbsoluteX(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedAbsoluteY: {
        u16 addr = address::IndexedAbsoluteY(cpu, cpu->ReadNext16());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedIndirectX: {
        u16 addr = ReadIndexedIndirectX(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      case AddressingMode::IndexedIndirectY: {
        u16 addr = ReadIndexedIndirectY(cpu, cpu->ReadNext());
        cpu->Tick();
        val = cpu->Read(addr);
        u8 new_c = val & 0b1;
        result = ((val >> 1) & 0x7F) | ((cpu->registers.p.carry & 0b1) << 7);
        cpu->registers.p.carry = new_c;
        cpu->Tick();
        cpu->Write(addr, result);
        break;
      }
      default: std::unreachable();
    }

    u8 orig_a = cpu->registers.a;
    u16 new_val = cpu->registers.a + result + cpu->registers.p.carry;
    cpu->registers.a = new_val & 0xFF;
    cpu->registers.p.carry = new_val > 0xFF;
    cpu->registers.p.zero = cpu->registers.a == 0;
    cpu->registers.p.overflow = (((cpu->registers.a ^ orig_a) & (cpu->registers.a ^ result)) >> 7) & 0b1;
    cpu->registers.p.negative = (cpu->registers.a >> 7) & 0b1;
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
    case Op::NOP: new_pc = exec::NOP(instr, this); break;
    case Op::JMP: new_pc = exec::JMP(instr, this); break;
    case Op::JSR: new_pc = exec::JSR(instr, this); break;
    case Op::RTS: new_pc = exec::RTS(instr, this); break;
    case Op::RTI: new_pc = exec::RTI(instr, this); break;
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
    case Op::LSR: new_pc = exec::LSR(instr, this); break;
    case Op::ASL: new_pc = exec::ASL(instr, this); break;
    case Op::ROR: new_pc = exec::ROR(instr, this); break;
    case Op::ROL: new_pc = exec::ROL(instr, this); break;
    case Op::INC: new_pc = exec::INC(instr, this); break;
    case Op::DEC: new_pc = exec::DEC(instr, this); break;
    case Op::LAX: new_pc = exec::LAX(instr, this); break;
    case Op::SAX: new_pc = exec::SAX(instr, this); break;
    case Op::DCP: new_pc = exec::DCP(instr, this); break;
    case Op::ISB: new_pc = exec::ISB(instr, this); break;
    case Op::SLO: new_pc = exec::SLO(instr, this); break;
    case Op::RLA: new_pc = exec::RLA(instr, this); break;
    case Op::SRE: new_pc = exec::SRE(instr, this); break;
    case Op::RRA: new_pc = exec::RRA(instr, this); break;
    default:
      spdlog::warn("Instruction not implemented: {}({:02X}) @ 0x{:02X}", magic_enum::enum_name(instr.op), instr.code, instr.addr);
  }

  spdlog::trace("Setting new PC = {:02X}", new_pc);
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

u8 Cpu::Increment(u16 address) {
  u8 result = bus->Read(address) + 1;
  spdlog::trace("Increment @{:04X} : {:02X} -> {:02X}", address, result-1, result);
  bus->Write(address, result);
  Tick();
  return result;
}

u8 Cpu::Decrement(u16 address) {
  u8 result = bus->Read(address) - 1;
  spdlog::trace("Decrement @{:04X} : {:02X} -> {:02X}", address, result+1, result);
  bus->Write(address, result);
  Tick();
  return result;
}

void Cpu::Tick() {
  cycles++;
}
