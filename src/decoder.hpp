#pragma once

#include "instruction.hpp"
#include "types.hpp"


class Decoder {
public:
  static Instruction Decode(u8 byte, u16 addr);
};
