#pragma once

#include <algorithm>
#include <array>
#include <span>

#include "op.hpp"
#include "types.hpp"


enum class AddressingMode {
  Implicit,
  Accumulator,
  Immediate,
  ZeroPage,
  Absolute,
  Relative,
  Indirect,
  IndexedZeroPageX,
  IndexedZeroPageY,
  IndexedAbsoluteX,
  IndexedAbsoluteY,
  IndexedIndirectX,
  IndexedIndirectY,
};

struct Instruction {
  u16 addr;
  u8 code;
  u8 lo;
  u8 hi;
  Op op;
  AddressingMode addressing_mode;
  u8 num_bytes;

  static auto From(Op op, AddressingMode mode, u8 byte, u16 addr, u8 skip_bytes = 0) {
    u8 num_bytes;
    if (mode == AddressingMode::Implicit) {
      num_bytes = 1;
    } else if (mode == AddressingMode::Absolute || mode == AddressingMode::IndexedAbsoluteX || mode == AddressingMode::IndexedAbsoluteY) {
      num_bytes = 3;
    } else {
      num_bytes = 2;
    }

    Instruction instr {
      .addr = addr,
      .code = byte,
      .lo = 0,
      .hi = 0,
      .op = op,
      .addressing_mode = mode,
      .num_bytes = static_cast<u8>(num_bytes + skip_bytes),
    };

    return instr;
  }

  static auto Unknown(u8 byte, u16 addr) {
    return Instruction { .code = byte, .addr = addr, .op = Op::UNKNOWN };
  }
};
