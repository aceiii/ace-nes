#include "decoder.hpp"

Instruction Decoder::Decode(const u8* mem) {
  switch (*mem)
  {
    // ADC
    case 0x69: return Instruction::From(Op::ADC, AddressingMode::Immediate, mem);
    case 0x65: return Instruction::From(Op::ADC, AddressingMode::ZeroPage, mem);
    case 0x75: return Instruction::From(Op::ADC, AddressingMode::IndexedZeroPageX, mem);
    case 0x6D: return Instruction::From(Op::ADC, AddressingMode::Absolute, mem);
    case 0x7D: return Instruction::From(Op::ADC, AddressingMode::IndexedAbsoluteX, mem);
    case 0x79: return Instruction::From(Op::ADC, AddressingMode::IndexedAbsoluteY, mem);
    case 0x61: return Instruction::From(Op::ADC, AddressingMode::IndexedIndirectX, mem);
    case 0x71: return Instruction::From(Op::ADC, AddressingMode::IndexedIndirectY, mem);

    // AND
    case 0x29: return Instruction::From(Op::AND, AddressingMode::Immediate, mem);
    case 0x25: return Instruction::From(Op::AND, AddressingMode::ZeroPage, mem);
    case 0x35: return Instruction::From(Op::AND, AddressingMode::IndexedZeroPageX, mem);
    case 0x2D: return Instruction::From(Op::AND, AddressingMode::Absolute, mem);
    case 0x3D: return Instruction::From(Op::AND, AddressingMode::IndexedAbsoluteX, mem);
    case 0x39: return Instruction::From(Op::AND, AddressingMode::IndexedAbsoluteY, mem);
    case 0x21: return Instruction::From(Op::AND, AddressingMode::IndexedIndirectX, mem);
    case 0x31: return Instruction::From(Op::AND, AddressingMode::IndexedIndirectY, mem);

    // ASL
    case 0x0A: return Instruction::From(Op::ASL, AddressingMode::Accumulator, mem);
    case 0x06: return Instruction::From(Op::ASL, AddressingMode::ZeroPage, mem);
    case 0x16: return Instruction::From(Op::ASL, AddressingMode::IndexedZeroPageX, mem);
    case 0x0E: return Instruction::From(Op::ASL, AddressingMode::Absolute, mem);
    case 0x1E: return Instruction::From(Op::ASL, AddressingMode::IndexedAbsoluteX, mem);

    // BCC
    case 0x90: return Instruction::From(Op::BCC, AddressingMode::Relative, mem);

    // BCS
    case 0xB0: return Instruction::From(Op::BCS, AddressingMode::Relative, mem);

    // BEQ
    case 0xF0: return Instruction::From(Op::BEQ, AddressingMode::Relative, mem);

    // BIT
    case 0x24: return Instruction::From(Op::BIT, AddressingMode::ZeroPage, mem);
    case 0x2C: return Instruction::From(Op::BIT, AddressingMode::Absolute, mem);

    // BMI
    case 0x30: return Instruction::From(Op::BMI, AddressingMode::Relative, mem);

    // BNE
    case 0xD0: return Instruction::From(Op::BNE, AddressingMode::Relative, mem);

    // BPL
    case 0x10: return Instruction::From(Op::BPL, AddressingMode::Relative, mem);

    // BRK
    case 0x00: return Instruction::From(Op::BRK, AddressingMode::Implicit, mem, 1);

    // BVC
    case 0x50: return Instruction::From(Op::BVC, AddressingMode::Relative, mem);

    // BVS
    case 0x70: return Instruction::From(Op::BVS, AddressingMode::Relative, mem);

    // CLC
    case 0x18: return Instruction::From(Op::CLC, AddressingMode::Implicit, mem);

    // CLD
    case 0xD8: return Instruction::From(Op::CLD, AddressingMode::Implicit, mem);

    // CLI
    case 0x58: return Instruction::From(Op::CLI, AddressingMode::Implicit, mem);

    // CLV
    case 0xB8: return Instruction::From(Op::CLV, AddressingMode::Implicit, mem);

    // CMP
    case 0xC9: return Instruction::From(Op::CMP, AddressingMode::Immediate, mem);
    case 0xC5: return Instruction::From(Op::CMP, AddressingMode::ZeroPage, mem);
    case 0xD5: return Instruction::From(Op::CMP, AddressingMode::IndexedZeroPageX, mem);
    case 0xCD: return Instruction::From(Op::CMP, AddressingMode::Absolute, mem);
    case 0xDD: return Instruction::From(Op::CMP, AddressingMode::IndexedAbsoluteX, mem);
    case 0xD9: return Instruction::From(Op::CMP, AddressingMode::IndexedAbsoluteY, mem);
    case 0xC1: return Instruction::From(Op::CMP, AddressingMode::IndexedZeroPageX, mem);
    case 0xD1: return Instruction::From(Op::CMP, AddressingMode::IndexedZeroPageY, mem);

    // CPX
    case 0xE0: return Instruction::From(Op::CPX, AddressingMode::Immediate, mem);
    case 0xE4: return Instruction::From(Op::CPX, AddressingMode::ZeroPage, mem);
    case 0xEC: return Instruction::From(Op::CPX, AddressingMode::Absolute, mem);

    // CPY
    case 0xC0: return Instruction::From(Op::CPY, AddressingMode::Immediate, mem);
    case 0xC4: return Instruction::From(Op::CPY, AddressingMode::ZeroPage, mem);
    case 0xCC: return Instruction::From(Op::CPY, AddressingMode::Absolute, mem);

    // DEC
    case 0xC6: return Instruction::From(Op::DEC, AddressingMode::ZeroPage, mem);
    case 0xD6: return Instruction::From(Op::DEC, AddressingMode::IndexedZeroPageX, mem);
    case 0xCE: return Instruction::From(Op::DEC, AddressingMode::Absolute, mem);
    case 0xDE: return Instruction::From(Op::DEC, AddressingMode::IndexedAbsoluteX, mem);

    // DEX
    case 0xCA: return Instruction::From(Op::DEX, AddressingMode::Implicit, mem);

    // DEY
    case 0x88: return Instruction::From(Op::DEY, AddressingMode::Implicit, mem);

    // EOR
    case 0x49: return Instruction::From(Op::EOR, AddressingMode::Immediate, mem);
    case 0x45: return Instruction::From(Op::EOR, AddressingMode::ZeroPage, mem);
    case 0x55: return Instruction::From(Op::EOR, AddressingMode::IndexedZeroPageX, mem);
    case 0x4D: return Instruction::From(Op::EOR, AddressingMode::Absolute, mem);
    case 0x5D: return Instruction::From(Op::EOR, AddressingMode::IndexedAbsoluteX, mem);
    case 0x59: return Instruction::From(Op::EOR, AddressingMode::IndexedAbsoluteY, mem);
    case 0x41: return Instruction::From(Op::EOR, AddressingMode::IndexedIndirectX, mem);
    case 0x51: return Instruction::From(Op::EOR, AddressingMode::IndexedIndirectY, mem);

    // INC
    case 0xE6: return Instruction::From(Op::INC, AddressingMode::ZeroPage, mem);
    case 0xF6: return Instruction::From(Op::INC, AddressingMode::IndexedZeroPageX, mem);
    case 0xEE: return Instruction::From(Op::INC, AddressingMode::Absolute, mem);
    case 0xFE: return Instruction::From(Op::INC, AddressingMode::IndexedAbsoluteX, mem);

    // INX
    case 0xE8: return Instruction::From(Op::INX, AddressingMode::Implicit, mem);

    // INY
    case 0xC8: return Instruction::From(Op::INY, AddressingMode::Implicit, mem);

    // JMP
    case 0x4C: return Instruction::From(Op::JMP, AddressingMode::Absolute, mem);
    case 0x6C: return Instruction::From(Op::JMP, AddressingMode::Indirect, mem);

    // JSR
    case 0x20: return Instruction::From(Op::JSR, AddressingMode::Absolute, mem);

    // LDA
    case 0xA9: return Instruction::From(Op::LDA, AddressingMode::Immediate, mem);
    case 0xA5: return Instruction::From(Op::LDA, AddressingMode::ZeroPage, mem);
    case 0xB5: return Instruction::From(Op::LDA, AddressingMode::IndexedZeroPageX, mem);
    case 0xAD: return Instruction::From(Op::LDA, AddressingMode::Absolute, mem);
    case 0xBD: return Instruction::From(Op::LDA, AddressingMode::IndexedAbsoluteX, mem);
    case 0xB9: return Instruction::From(Op::LDA, AddressingMode::IndexedAbsoluteY, mem);
    case 0xA1: return Instruction::From(Op::LDA, AddressingMode::IndexedIndirectX, mem);
    case 0xB1: return Instruction::From(Op::LDA, AddressingMode::IndexedIndirectY, mem);

    // LDX
    case 0xA2: return Instruction::From(Op::LDX, AddressingMode::Immediate, mem);
    case 0xA6: return Instruction::From(Op::LDX, AddressingMode::ZeroPage, mem);
    case 0xB6: return Instruction::From(Op::LDX, AddressingMode::IndexedZeroPageY, mem);
    case 0xAE: return Instruction::From(Op::LDX, AddressingMode::Absolute, mem);
    case 0xBE: return Instruction::From(Op::LDX, AddressingMode::IndexedAbsoluteY, mem);

    // LDY
    case 0xA0: return Instruction::From(Op::LDY, AddressingMode::Immediate, mem);
    case 0xA4: return Instruction::From(Op::LDY, AddressingMode::ZeroPage, mem);
    case 0xB4: return Instruction::From(Op::LDY, AddressingMode::IndexedZeroPageX, mem);
    case 0xAC: return Instruction::From(Op::LDY, AddressingMode::Absolute, mem);
    case 0xBC: return Instruction::From(Op::LDY, AddressingMode::IndexedAbsoluteX, mem);

    // LSR
    case 0x4A: return Instruction::From(Op::LSR, AddressingMode::Accumulator, mem);
    case 0x46: return Instruction::From(Op::LSR, AddressingMode::ZeroPage, mem);
    case 0x56: return Instruction::From(Op::LSR, AddressingMode::IndexedZeroPageX, mem);
    case 0x4E: return Instruction::From(Op::LSR, AddressingMode::Absolute, mem);
    case 0x5E: return Instruction::From(Op::LSR, AddressingMode::IndexedAbsoluteX, mem);

    // NOP
    case 0xEA: return Instruction::From(Op::NOP, AddressingMode::Implicit, mem);

    // ORA
    case 0x09: return Instruction::From(Op::ORA, AddressingMode::Immediate, mem);
    case 0x05: return Instruction::From(Op::ORA, AddressingMode::ZeroPage, mem);
    case 0x15: return Instruction::From(Op::ORA, AddressingMode::IndexedZeroPageX, mem);
    case 0x0D: return Instruction::From(Op::ORA, AddressingMode::Absolute, mem);
    case 0x1D: return Instruction::From(Op::ORA, AddressingMode::IndexedAbsoluteX, mem);
    case 0x19: return Instruction::From(Op::ORA, AddressingMode::IndexedAbsoluteY, mem);
    case 0x01: return Instruction::From(Op::ORA, AddressingMode::IndexedIndirectX, mem);
    case 0x11: return Instruction::From(Op::ORA, AddressingMode::IndexedIndirectY, mem);

    // PHA
    case 0x48: return Instruction::From(Op::PHA, AddressingMode::Implicit, mem);

    // PHP
    case 0x08: return Instruction::From(Op::PHP, AddressingMode::Implicit, mem);

    // PLA
    case 0x68: return Instruction::From(Op::PLA, AddressingMode::Implicit, mem);

    // PLP
    case 0x28: return Instruction::From(Op::PLP, AddressingMode::Implicit, mem);

    // ROL
    case 0x2A: return Instruction::From(Op::ROL, AddressingMode::Accumulator, mem);
    case 0x26: return Instruction::From(Op::ROL, AddressingMode::ZeroPage, mem);
    case 0x36: return Instruction::From(Op::ROL, AddressingMode::IndexedZeroPageX, mem);
    case 0x2E: return Instruction::From(Op::ROL, AddressingMode::Absolute, mem);
    case 0x3E: return Instruction::From(Op::ROL, AddressingMode::IndexedAbsoluteX, mem);

    // ROR
    case 0x6A: return Instruction::From(Op::ROR, AddressingMode::Accumulator, mem);
    case 0x66: return Instruction::From(Op::ROR, AddressingMode::ZeroPage, mem);
    case 0x76: return Instruction::From(Op::ROR, AddressingMode::IndexedZeroPageX, mem);
    case 0x6E: return Instruction::From(Op::ROR, AddressingMode::Absolute, mem);
    case 0x7E: return Instruction::From(Op::ROR, AddressingMode::IndexedAbsoluteX, mem);

    // RTI
    case 0x40: return Instruction::From(Op::RTI, AddressingMode::Implicit, mem);

    // RTS
    case 0x60: return Instruction::From(Op::RTS, AddressingMode::Implicit, mem);

    // SBC
    case 0xE9: return Instruction::From(Op::SBC, AddressingMode::Immediate, mem);
    case 0xE5: return Instruction::From(Op::SBC, AddressingMode::ZeroPage, mem);
    case 0xF5: return Instruction::From(Op::SBC, AddressingMode::IndexedZeroPageX, mem);
    case 0xED: return Instruction::From(Op::SBC, AddressingMode::Absolute, mem);
    case 0xFD: return Instruction::From(Op::SBC, AddressingMode::IndexedAbsoluteX, mem);
    case 0xF9: return Instruction::From(Op::SBC, AddressingMode::IndexedAbsoluteY, mem);
    case 0xE1: return Instruction::From(Op::SBC, AddressingMode::IndexedIndirectX, mem);
    case 0xF1: return Instruction::From(Op::SBC, AddressingMode::IndexedIndirectY, mem);

    // SEC
    case 0x38: return Instruction::From(Op::SEC, AddressingMode::Implicit, mem);

    // SED
    case 0xF8: return Instruction::From(Op::SED, AddressingMode::Implicit, mem);

    // SEI
    case 0x78: return Instruction::From(Op::SEI, AddressingMode::Implicit, mem);

    // STA
    case 0x85: return Instruction::From(Op::STA, AddressingMode::ZeroPage, mem);
    case 0x95: return Instruction::From(Op::STA, AddressingMode::IndexedZeroPageX, mem);
    case 0x8D: return Instruction::From(Op::STA, AddressingMode::Absolute, mem);
    case 0x9D: return Instruction::From(Op::STA, AddressingMode::IndexedAbsoluteX, mem);
    case 0x99: return Instruction::From(Op::STA, AddressingMode::IndexedAbsoluteY, mem);
    case 0x81: return Instruction::From(Op::STA, AddressingMode::IndexedIndirectY, mem);
    case 0x91: return Instruction::From(Op::STA, AddressingMode::IndexedIndirectX, mem);

    // STX
    case 0x86: return Instruction::From(Op::STX, AddressingMode::ZeroPage, mem);
    case 0x96: return Instruction::From(Op::STX, AddressingMode::IndexedZeroPageY, mem);
    case 0x8E: return Instruction::From(Op::STX, AddressingMode::Absolute, mem);

    // STY
    case 0x84: return Instruction::From(Op::STY, AddressingMode::ZeroPage, mem);
    case 0x94: return Instruction::From(Op::STY, AddressingMode::IndexedZeroPageX, mem);
    case 0x8C: return Instruction::From(Op::STY, AddressingMode::Absolute, mem);

    // TAX
    case 0xAA: return Instruction::From(Op::TAX, AddressingMode::Implicit, mem);

    // TAY
    case 0xA8: return Instruction::From(Op::TAY, AddressingMode::Implicit, mem);

    // TSX
    case 0xBA: return Instruction::From(Op::TSX, AddressingMode::Implicit, mem);

    // TXA
    case 0x8A: return Instruction::From(Op::TXA, AddressingMode::Implicit, mem);

    // TXS
    case 0x9A: return Instruction::From(Op::TXS, AddressingMode::Implicit, mem);

    // TYA
    case 0x98: return Instruction::From(Op::TYA, AddressingMode::Implicit, mem);
  }

  return Instruction::Unknown();
}
