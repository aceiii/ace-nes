#include <functional>
#include <unordered_map>
#include <spdlog/spdlog.h>

#include "decoder.hpp"


Instruction Decoder::Decode(u8 byte, u16 addr) {
  static const std::unordered_map<u8, std::function<Instruction(void)>> decoder_map {
    // ADC
    { 0x69, [&](){ return Instruction::From(Op::ADC, AddressingMode::Immediate, byte, addr); } },
    { 0x65, [&](){ return Instruction::From(Op::ADC, AddressingMode::ZeroPage, byte, addr); } },
    { 0x75, [&](){ return Instruction::From(Op::ADC, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x6D, [&](){ return Instruction::From(Op::ADC, AddressingMode::Absolute, byte, addr); } },
    { 0x7D, [&](){ return Instruction::From(Op::ADC, AddressingMode::IndexedAbsoluteX, byte, addr); } },
    { 0x79, [&](){ return Instruction::From(Op::ADC, AddressingMode::IndexedAbsoluteY, byte, addr); } },
    { 0x61, [&](){ return Instruction::From(Op::ADC, AddressingMode::IndexedIndirectX, byte, addr); } },
    { 0x71, [&](){ return Instruction::From(Op::ADC, AddressingMode::IndexedIndirectY, byte, addr); } },

    // AND
    { 0x29, [&](){ return Instruction::From(Op::AND, AddressingMode::Immediate, byte, addr); } },
    { 0x25, [&](){ return Instruction::From(Op::AND, AddressingMode::ZeroPage, byte, addr); } },
    { 0x35, [&](){ return Instruction::From(Op::AND, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x2D, [&](){ return Instruction::From(Op::AND, AddressingMode::Absolute, byte, addr); } },
    { 0x3D, [&](){ return Instruction::From(Op::AND, AddressingMode::IndexedAbsoluteX, byte, addr); } },
    { 0x39, [&](){ return Instruction::From(Op::AND, AddressingMode::IndexedAbsoluteY, byte, addr); } },
    { 0x21, [&](){ return Instruction::From(Op::AND, AddressingMode::IndexedIndirectX, byte, addr); } },
    { 0x31, [&](){ return Instruction::From(Op::AND, AddressingMode::IndexedIndirectY, byte, addr); } },

    // ASL
    { 0x0A, [&](){ return Instruction::From(Op::ASL, AddressingMode::Accumulator, byte, addr); } },
    { 0x06, [&](){ return Instruction::From(Op::ASL, AddressingMode::ZeroPage, byte, addr); } },
    { 0x16, [&](){ return Instruction::From(Op::ASL, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x0E, [&](){ return Instruction::From(Op::ASL, AddressingMode::Absolute, byte, addr); } },
    { 0x1E, [&](){ return Instruction::From(Op::ASL, AddressingMode::IndexedAbsoluteX, byte, addr); } },

    // BCC
    { 0x90, [&](){ return Instruction::From(Op::BCC, AddressingMode::Relative, byte, addr); } },

    // BCS
    { 0xB0, [&](){ return Instruction::From(Op::BCS, AddressingMode::Relative, byte, addr); } },

    // BEQ
    { 0xF0, [&](){ return Instruction::From(Op::BEQ, AddressingMode::Relative, byte, addr); } },

    // BIT
    { 0x24, [&](){ return Instruction::From(Op::BIT, AddressingMode::ZeroPage, byte, addr); } },
    { 0x2C, [&](){ return Instruction::From(Op::BIT, AddressingMode::Absolute, byte, addr); } },

    // BMI
    { 0x30, [&](){ return Instruction::From(Op::BMI, AddressingMode::Relative, byte, addr); } },

    // BNE
    { 0xD0, [&](){ return Instruction::From(Op::BNE, AddressingMode::Relative, byte, addr); } },

    // BPL
    { 0x10, [&](){ return Instruction::From(Op::BPL, AddressingMode::Relative, byte, addr); } },

    // BRK
    { 0x00, [&](){ return Instruction::From(Op::BRK, AddressingMode::Implicit, byte, 1); } },

    // BVC
    { 0x50, [&](){ return Instruction::From(Op::BVC, AddressingMode::Relative, byte, addr); } },

    // BVS
    { 0x70, [&](){ return Instruction::From(Op::BVS, AddressingMode::Relative, byte, addr); } },

    // CLC
    { 0x18, [&](){ return Instruction::From(Op::CLC, AddressingMode::Implicit, byte, addr); } },

    // CLD
    { 0xD8, [&](){ return Instruction::From(Op::CLD, AddressingMode::Implicit, byte, addr); } },

    // CLI
    { 0x58, [&](){ return Instruction::From(Op::CLI, AddressingMode::Implicit, byte, addr); } },

    // CLV
    { 0xB8, [&](){ return Instruction::From(Op::CLV, AddressingMode::Implicit, byte, addr); } },

    // CMP
    { 0xC9, [&](){ return Instruction::From(Op::CMP, AddressingMode::Immediate, byte, addr); } },
    { 0xC5, [&](){ return Instruction::From(Op::CMP, AddressingMode::ZeroPage, byte, addr); } },
    { 0xD5, [&](){ return Instruction::From(Op::CMP, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0xCD, [&](){ return Instruction::From(Op::CMP, AddressingMode::Absolute, byte, addr); } },
    { 0xDD, [&](){ return Instruction::From(Op::CMP, AddressingMode::IndexedAbsoluteX, byte, addr); } },
    { 0xD9, [&](){ return Instruction::From(Op::CMP, AddressingMode::IndexedAbsoluteY, byte, addr); } },
    { 0xC1, [&](){ return Instruction::From(Op::CMP, AddressingMode::IndexedIndirectX, byte, addr); } },
    { 0xD1, [&](){ return Instruction::From(Op::CMP, AddressingMode::IndexedIndirectY, byte, addr); } },

    // CPX
    { 0xE0, [&](){ return Instruction::From(Op::CPX, AddressingMode::Immediate, byte, addr); } },
    { 0xE4, [&](){ return Instruction::From(Op::CPX, AddressingMode::ZeroPage, byte, addr); } },
    { 0xEC, [&](){ return Instruction::From(Op::CPX, AddressingMode::Absolute, byte, addr); } },

    // CPY
    { 0xC0, [&](){ return Instruction::From(Op::CPY, AddressingMode::Immediate, byte, addr); } },
    { 0xC4, [&](){ return Instruction::From(Op::CPY, AddressingMode::ZeroPage, byte, addr); } },
    { 0xCC, [&](){ return Instruction::From(Op::CPY, AddressingMode::Absolute, byte, addr); } },

    // DEC
    { 0xC6, [&](){ return Instruction::From(Op::DEC, AddressingMode::ZeroPage, byte, addr); } },
    { 0xD6, [&](){ return Instruction::From(Op::DEC, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0xCE, [&](){ return Instruction::From(Op::DEC, AddressingMode::Absolute, byte, addr); } },
    { 0xDE, [&](){ return Instruction::From(Op::DEC, AddressingMode::IndexedAbsoluteX, byte, addr); } },

    // DEX
    { 0xCA, [&](){ return Instruction::From(Op::DEX, AddressingMode::Implicit, byte, addr); } },

    // DEY
    { 0x88, [&](){ return Instruction::From(Op::DEY, AddressingMode::Implicit, byte, addr); } },

    // EOR
    { 0x49, [&](){ return Instruction::From(Op::EOR, AddressingMode::Immediate, byte, addr); } },
    { 0x45, [&](){ return Instruction::From(Op::EOR, AddressingMode::ZeroPage, byte, addr); } },
    { 0x55, [&](){ return Instruction::From(Op::EOR, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x4D, [&](){ return Instruction::From(Op::EOR, AddressingMode::Absolute, byte, addr); } },
    { 0x5D, [&](){ return Instruction::From(Op::EOR, AddressingMode::IndexedAbsoluteX, byte, addr); } },
    { 0x59, [&](){ return Instruction::From(Op::EOR, AddressingMode::IndexedAbsoluteY, byte, addr); } },
    { 0x41, [&](){ return Instruction::From(Op::EOR, AddressingMode::IndexedIndirectX, byte, addr); } },
    { 0x51, [&](){ return Instruction::From(Op::EOR, AddressingMode::IndexedIndirectY, byte, addr); } },

    // INC
    { 0xE6, [&](){ return Instruction::From(Op::INC, AddressingMode::ZeroPage, byte, addr); } },
    { 0xF6, [&](){ return Instruction::From(Op::INC, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0xEE, [&](){ return Instruction::From(Op::INC, AddressingMode::Absolute, byte, addr); } },
    { 0xFE, [&](){ return Instruction::From(Op::INC, AddressingMode::IndexedAbsoluteX, byte, addr); } },

    // INX
    { 0xE8, [&](){ return Instruction::From(Op::INX, AddressingMode::Implicit, byte, addr); } },

    // INY
    { 0xC8, [&](){ return Instruction::From(Op::INY, AddressingMode::Implicit, byte, addr); } },

    // JMP
    { 0x4C, [&](){ return Instruction::From(Op::JMP, AddressingMode::Absolute, byte, addr); } },
    { 0x6C, [&](){ return Instruction::From(Op::JMP, AddressingMode::Indirect, byte, addr); } },

    // JSR
    { 0x20, [&](){ return Instruction::From(Op::JSR, AddressingMode::Absolute, byte, addr); } },

    // LDA
    { 0xA9, [&](){ return Instruction::From(Op::LDA, AddressingMode::Immediate, byte, addr); } },
    { 0xA5, [&](){ return Instruction::From(Op::LDA, AddressingMode::ZeroPage, byte, addr); } },
    { 0xB5, [&](){ return Instruction::From(Op::LDA, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0xAD, [&](){ return Instruction::From(Op::LDA, AddressingMode::Absolute, byte, addr); } },
    { 0xBD, [&](){ return Instruction::From(Op::LDA, AddressingMode::IndexedAbsoluteX, byte, addr); } },
    { 0xB9, [&](){ return Instruction::From(Op::LDA, AddressingMode::IndexedAbsoluteY, byte, addr); } },
    { 0xA1, [&](){ return Instruction::From(Op::LDA, AddressingMode::IndexedIndirectX, byte, addr); } },
    { 0xB1, [&](){ return Instruction::From(Op::LDA, AddressingMode::IndexedIndirectY, byte, addr); } },

    // LDX
    { 0xA2, [&](){ return Instruction::From(Op::LDX, AddressingMode::Immediate, byte, addr); } },
    { 0xA6, [&](){ return Instruction::From(Op::LDX, AddressingMode::ZeroPage, byte, addr); } },
    { 0xB6, [&](){ return Instruction::From(Op::LDX, AddressingMode::IndexedZeroPageY, byte, addr); } },
    { 0xAE, [&](){ return Instruction::From(Op::LDX, AddressingMode::Absolute, byte, addr); } },
    { 0xBE, [&](){ return Instruction::From(Op::LDX, AddressingMode::IndexedAbsoluteY, byte, addr); } },

    // LDY
    { 0xA0, [&](){ return Instruction::From(Op::LDY, AddressingMode::Immediate, byte, addr); } },
    { 0xA4, [&](){ return Instruction::From(Op::LDY, AddressingMode::ZeroPage, byte, addr); } },
    { 0xB4, [&](){ return Instruction::From(Op::LDY, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0xAC, [&](){ return Instruction::From(Op::LDY, AddressingMode::Absolute, byte, addr); } },
    { 0xBC, [&](){ return Instruction::From(Op::LDY, AddressingMode::IndexedAbsoluteX, byte, addr); } },

    // LSR
    { 0x4A, [&](){ return Instruction::From(Op::LSR, AddressingMode::Accumulator, byte, addr); } },
    { 0x46, [&](){ return Instruction::From(Op::LSR, AddressingMode::ZeroPage, byte, addr); } },
    { 0x56, [&](){ return Instruction::From(Op::LSR, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x4E, [&](){ return Instruction::From(Op::LSR, AddressingMode::Absolute, byte, addr); } },
    { 0x5E, [&](){ return Instruction::From(Op::LSR, AddressingMode::IndexedAbsoluteX, byte, addr); } },

    // NOP
    { 0xEA, [&](){ return Instruction::From(Op::NOP, AddressingMode::Implicit, byte, addr); } },
    // unofficial NOP
    { 0x1A, [&](){ return Instruction::From(Op::NOP, AddressingMode::Implicit, byte, addr, 0, true); } },
    { 0x3A, [&](){ return Instruction::From(Op::NOP, AddressingMode::Implicit, byte, addr, 0, true); } },
    { 0x5A, [&](){ return Instruction::From(Op::NOP, AddressingMode::Implicit, byte, addr, 0, true); } },
    { 0x7A, [&](){ return Instruction::From(Op::NOP, AddressingMode::Implicit, byte, addr, 0, true); } },
    { 0xDA, [&](){ return Instruction::From(Op::NOP, AddressingMode::Implicit, byte, addr, 0, true); } },
    { 0xFA, [&](){ return Instruction::From(Op::NOP, AddressingMode::Implicit, byte, addr, 0, true); } },
    // SKB #i
    { 0x80, [&](){ return Instruction::From(Op::NOP, AddressingMode::Immediate, byte, addr, 0, true); } },
    { 0x82, [&](){ return Instruction::From(Op::NOP, AddressingMode::Immediate, byte, addr, 0, true); } },
    { 0x89, [&](){ return Instruction::From(Op::NOP, AddressingMode::Immediate, byte, addr, 0, true); } },
    { 0xC2, [&](){ return Instruction::From(Op::NOP, AddressingMode::Immediate, byte, addr, 0, true); } },
    { 0xE2, [&](){ return Instruction::From(Op::NOP, AddressingMode::Immediate, byte, addr, 0, true); } },
    // IGN a
    { 0x0C, [&](){ return Instruction::From(Op::NOP, AddressingMode::Absolute, byte, addr, 0, true); } },
    // IGN a,X
    { 0x1C, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },
    { 0x3C, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },
    { 0x5C, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },
    { 0x7C, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },
    { 0xDC, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },
    { 0xFC, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },
    // IGN d
    { 0x04, [&](){ return Instruction::From(Op::NOP, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0x44, [&](){ return Instruction::From(Op::NOP, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0x64, [&](){ return Instruction::From(Op::NOP, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    // IGN d,X
    { 0x14, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0x34, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0x54, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0x74, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0xD4, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0xF4, [&](){ return Instruction::From(Op::NOP, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },

    // ORA
    { 0x09, [&](){ return Instruction::From(Op::ORA, AddressingMode::Immediate, byte, addr); } },
    { 0x05, [&](){ return Instruction::From(Op::ORA, AddressingMode::ZeroPage, byte, addr); } },
    { 0x15, [&](){ return Instruction::From(Op::ORA, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x0D, [&](){ return Instruction::From(Op::ORA, AddressingMode::Absolute, byte, addr); } },
    { 0x1D, [&](){ return Instruction::From(Op::ORA, AddressingMode::IndexedAbsoluteX, byte, addr); } },
    { 0x19, [&](){ return Instruction::From(Op::ORA, AddressingMode::IndexedAbsoluteY, byte, addr); } },
    { 0x01, [&](){ return Instruction::From(Op::ORA, AddressingMode::IndexedIndirectX, byte, addr); } },
    { 0x11, [&](){ return Instruction::From(Op::ORA, AddressingMode::IndexedIndirectY, byte, addr); } },

    // PHA
    { 0x48, [&](){ return Instruction::From(Op::PHA, AddressingMode::Implicit, byte, addr); } },

    // PHP
    { 0x08, [&](){ return Instruction::From(Op::PHP, AddressingMode::Implicit, byte, addr); } },

    // PLA
    { 0x68, [&](){ return Instruction::From(Op::PLA, AddressingMode::Implicit, byte, addr); } },

    // PLP
    { 0x28, [&](){ return Instruction::From(Op::PLP, AddressingMode::Implicit, byte, addr); } },

    // ROL
    { 0x2A, [&](){ return Instruction::From(Op::ROL, AddressingMode::Accumulator, byte, addr); } },
    { 0x26, [&](){ return Instruction::From(Op::ROL, AddressingMode::ZeroPage, byte, addr); } },
    { 0x36, [&](){ return Instruction::From(Op::ROL, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x2E, [&](){ return Instruction::From(Op::ROL, AddressingMode::Absolute, byte, addr); } },
    { 0x3E, [&](){ return Instruction::From(Op::ROL, AddressingMode::IndexedAbsoluteX, byte, addr); } },

    // ROR
    { 0x6A, [&](){ return Instruction::From(Op::ROR, AddressingMode::Accumulator, byte, addr); } },
    { 0x66, [&](){ return Instruction::From(Op::ROR, AddressingMode::ZeroPage, byte, addr); } },
    { 0x76, [&](){ return Instruction::From(Op::ROR, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x6E, [&](){ return Instruction::From(Op::ROR, AddressingMode::Absolute, byte, addr); } },
    { 0x7E, [&](){ return Instruction::From(Op::ROR, AddressingMode::IndexedAbsoluteX, byte, addr); } },

    // RTI
    { 0x40, [&](){ return Instruction::From(Op::RTI, AddressingMode::Implicit, byte, addr); } },

    // RTS
    { 0x60, [&](){ return Instruction::From(Op::RTS, AddressingMode::Implicit, byte, addr); } },

    // SBC
    { 0xE9, [&](){ return Instruction::From(Op::SBC, AddressingMode::Immediate, byte, addr); } },
    { 0xE5, [&](){ return Instruction::From(Op::SBC, AddressingMode::ZeroPage, byte, addr); } },
    { 0xF5, [&](){ return Instruction::From(Op::SBC, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0xED, [&](){ return Instruction::From(Op::SBC, AddressingMode::Absolute, byte, addr); } },
    { 0xFD, [&](){ return Instruction::From(Op::SBC, AddressingMode::IndexedAbsoluteX, byte, addr); } },
    { 0xF9, [&](){ return Instruction::From(Op::SBC, AddressingMode::IndexedAbsoluteY, byte, addr); } },
    { 0xE1, [&](){ return Instruction::From(Op::SBC, AddressingMode::IndexedIndirectX, byte, addr); } },
    { 0xF1, [&](){ return Instruction::From(Op::SBC, AddressingMode::IndexedIndirectY, byte, addr); } },
    { 0xEB, [&](){ return Instruction::From(Op::SBC, AddressingMode::Immediate, byte, addr, 0, true); } },

    // SEC
    { 0x38, [&](){ return Instruction::From(Op::SEC, AddressingMode::Implicit, byte, addr); } },

    // SED
    { 0xF8, [&](){ return Instruction::From(Op::SED, AddressingMode::Implicit, byte, addr); } },

    // SEI
    { 0x78, [&](){ return Instruction::From(Op::SEI, AddressingMode::Implicit, byte, addr); } },

    // STA
    { 0x85, [&](){ return Instruction::From(Op::STA, AddressingMode::ZeroPage, byte, addr); } },
    { 0x95, [&](){ return Instruction::From(Op::STA, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x8D, [&](){ return Instruction::From(Op::STA, AddressingMode::Absolute, byte, addr); } },
    { 0x9D, [&](){ return Instruction::From(Op::STA, AddressingMode::IndexedAbsoluteX, byte, addr); } },
    { 0x99, [&](){ return Instruction::From(Op::STA, AddressingMode::IndexedAbsoluteY, byte, addr); } },
    { 0x81, [&](){ return Instruction::From(Op::STA, AddressingMode::IndexedIndirectX, byte, addr); } },
    { 0x91, [&](){ return Instruction::From(Op::STA, AddressingMode::IndexedIndirectY, byte, addr); } },

    // STX
    { 0x86, [&](){ return Instruction::From(Op::STX, AddressingMode::ZeroPage, byte, addr); } },
    { 0x96, [&](){ return Instruction::From(Op::STX, AddressingMode::IndexedZeroPageY, byte, addr); } },
    { 0x8E, [&](){ return Instruction::From(Op::STX, AddressingMode::Absolute, byte, addr); } },

    // STY
    { 0x84, [&](){ return Instruction::From(Op::STY, AddressingMode::ZeroPage, byte, addr); } },
    { 0x94, [&](){ return Instruction::From(Op::STY, AddressingMode::IndexedZeroPageX, byte, addr); } },
    { 0x8C, [&](){ return Instruction::From(Op::STY, AddressingMode::Absolute, byte, addr); } },

    // TAX
    { 0xAA, [&](){ return Instruction::From(Op::TAX, AddressingMode::Implicit, byte, addr); } },

    // TAY
    { 0xA8, [&](){ return Instruction::From(Op::TAY, AddressingMode::Implicit, byte, addr); } },

    // TSX
    { 0xBA, [&](){ return Instruction::From(Op::TSX, AddressingMode::Implicit, byte, addr); } },

    // TXA
    { 0x8A, [&](){ return Instruction::From(Op::TXA, AddressingMode::Implicit, byte, addr); } },

    // TXS
    { 0x9A, [&](){ return Instruction::From(Op::TXS, AddressingMode::Implicit, byte, addr); } },

    // TYA
    { 0x98, [&](){ return Instruction::From(Op::TYA, AddressingMode::Implicit, byte, addr); } },

    // LAX
    { 0xA3, [&](){ return Instruction::From(Op::LAX, AddressingMode::IndexedIndirectX, byte, addr, 0, true); } },
    { 0xA7, [&](){ return Instruction::From(Op::LAX, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0xAF, [&](){ return Instruction::From(Op::LAX, AddressingMode::Absolute, byte, addr, 0, true); } },
    { 0xB3, [&](){ return Instruction::From(Op::LAX, AddressingMode::IndexedIndirectY, byte, addr, 0, true); } },
    { 0xB7, [&](){ return Instruction::From(Op::LAX, AddressingMode::IndexedZeroPageY, byte, addr, 0, true); } },
    { 0xBF, [&](){ return Instruction::From(Op::LAX, AddressingMode::IndexedAbsoluteY, byte, addr, 0, true); } },

    // SAX
    { 0x83, [&](){ return Instruction::From(Op::SAX, AddressingMode::IndexedIndirectX, byte, addr, 0, true); } },
    { 0x87, [&](){ return Instruction::From(Op::SAX, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0x8F, [&](){ return Instruction::From(Op::SAX, AddressingMode::Absolute, byte, addr, 0, true); } },
    { 0x97, [&](){ return Instruction::From(Op::SAX, AddressingMode::IndexedZeroPageY, byte, addr, 0, true); } },

    // DCP
    { 0xC3, [&](){ return Instruction::From(Op::DCP, AddressingMode::IndexedIndirectX, byte, addr, 0, true); } },
    { 0xC7, [&](){ return Instruction::From(Op::DCP, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0xCF, [&](){ return Instruction::From(Op::DCP, AddressingMode::Absolute, byte, addr, 0, true); } },
    { 0xD3, [&](){ return Instruction::From(Op::DCP, AddressingMode::IndexedIndirectY, byte, addr, 0, true); } },
    { 0xD7, [&](){ return Instruction::From(Op::DCP, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0xDB, [&](){ return Instruction::From(Op::DCP, AddressingMode::IndexedAbsoluteY, byte, addr, 0, true); } },
    { 0xDF, [&](){ return Instruction::From(Op::DCP, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },

    // ISB(ISC)
    { 0xE3, [&](){ return Instruction::From(Op::ISB, AddressingMode::IndexedIndirectX, byte, addr, 0, true); } },
    { 0xE7, [&](){ return Instruction::From(Op::ISB, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0xEF, [&](){ return Instruction::From(Op::ISB, AddressingMode::Absolute, byte, addr, 0, true); } },
    { 0xF3, [&](){ return Instruction::From(Op::ISB, AddressingMode::IndexedIndirectY, byte, addr, 0, true); } },
    { 0xF7, [&](){ return Instruction::From(Op::ISB, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0xFB, [&](){ return Instruction::From(Op::ISB, AddressingMode::IndexedAbsoluteY, byte, addr, 0, true); } },
    { 0xFF, [&](){ return Instruction::From(Op::ISB, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },

    // SLO
    { 0x03, [&](){ return Instruction::From(Op::SLO, AddressingMode::IndexedIndirectX, byte, addr, 0, true); } },
    { 0x07, [&](){ return Instruction::From(Op::SLO, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0x0F, [&](){ return Instruction::From(Op::SLO, AddressingMode::Absolute, byte, addr, 0, true); } },
    { 0x13, [&](){ return Instruction::From(Op::SLO, AddressingMode::IndexedIndirectY, byte, addr, 0, true); } },
    { 0x17, [&](){ return Instruction::From(Op::SLO, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0x1B, [&](){ return Instruction::From(Op::SLO, AddressingMode::IndexedAbsoluteY, byte, addr, 0, true); } },
    { 0x1F, [&](){ return Instruction::From(Op::SLO, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },

    // SRE
    { 0x43, [&](){ return Instruction::From(Op::SRE, AddressingMode::IndexedIndirectX, byte, addr, 0, true); } },
    { 0x47, [&](){ return Instruction::From(Op::SRE, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0x4F, [&](){ return Instruction::From(Op::SRE, AddressingMode::Absolute, byte, addr, 0, true); } },
    { 0x53, [&](){ return Instruction::From(Op::SRE, AddressingMode::IndexedIndirectY, byte, addr, 0, true); } },
    { 0x57, [&](){ return Instruction::From(Op::SRE, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0x5B, [&](){ return Instruction::From(Op::SRE, AddressingMode::IndexedAbsoluteY, byte, addr, 0, true); } },
    { 0x5F, [&](){ return Instruction::From(Op::SRE, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },

    // RLA
    { 0x23, [&](){ return Instruction::From(Op::RLA, AddressingMode::IndexedIndirectX, byte, addr, 0, true); } },
    { 0x27, [&](){ return Instruction::From(Op::RLA, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0x2F, [&](){ return Instruction::From(Op::RLA, AddressingMode::Absolute, byte, addr, 0, true); } },
    { 0x33, [&](){ return Instruction::From(Op::RLA, AddressingMode::IndexedIndirectY, byte, addr, 0, true); } },
    { 0x37, [&](){ return Instruction::From(Op::RLA, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0x3B, [&](){ return Instruction::From(Op::RLA, AddressingMode::IndexedAbsoluteY, byte, addr, 0, true); } },
    { 0x3F, [&](){ return Instruction::From(Op::RLA, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },

    // RRA
    { 0x63, [&](){ return Instruction::From(Op::RRA, AddressingMode::IndexedIndirectX, byte, addr, 0, true); } },
    { 0x67, [&](){ return Instruction::From(Op::RRA, AddressingMode::ZeroPage, byte, addr, 0, true); } },
    { 0x6F, [&](){ return Instruction::From(Op::RRA, AddressingMode::Absolute, byte, addr, 0, true); } },
    { 0x73, [&](){ return Instruction::From(Op::RRA, AddressingMode::IndexedIndirectY, byte, addr, 0, true); } },
    { 0x77, [&](){ return Instruction::From(Op::RRA, AddressingMode::IndexedZeroPageX, byte, addr, 0, true); } },
    { 0x7B, [&](){ return Instruction::From(Op::RRA, AddressingMode::IndexedAbsoluteY, byte, addr, 0, true); } },
    { 0x7F, [&](){ return Instruction::From(Op::RRA, AddressingMode::IndexedAbsoluteX, byte, addr, 0, true); } },
  };

  const auto decoder_it = decoder_map.find(byte);
  if (decoder_it != decoder_map.end()) {
    return decoder_it->second();
  }

  return Instruction::Unknown(byte, addr);
}
