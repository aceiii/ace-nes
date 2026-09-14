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
  u8 code;
  Op op;
  AddressingMode addressing_mode;
  u16 arg;
  u8 num_bytes;
  std::array<u8, 3> bytes;

  static auto From(Op op, AddressingMode mode, const u8* mem, u8 skip_bytes = 0) {
    u8 num_bytes;
    if (mode == AddressingMode::Implicit) {
      num_bytes = 1;
    } else if (mode == AddressingMode::Absolute || mode == AddressingMode::IndexedAbsoluteX || mode == AddressingMode::IndexedAbsoluteY) {
      num_bytes = 3;
    } else {
      num_bytes = 2;
    }

    auto bytes = std::span<const u8>(mem, num_bytes);

    u16 arg;
    if (num_bytes == 3) {
      arg = bytes[1] | (bytes[2] << 8);
    } else if (num_bytes == 2) {
      arg = bytes[1];
    }

    Instruction instr {
      .code = bytes[0],
      .op = op,
      .addressing_mode = mode,
      .arg = arg,
      .num_bytes = static_cast<u8>(num_bytes + skip_bytes),
    };

    std::ranges::copy(bytes, instr.bytes.begin());

    return instr;
  }

  static auto Unknown() {
    return Instruction { .op = Op::UNKNOWN };
  }
};
