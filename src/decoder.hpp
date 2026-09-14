#pragma once

#include "instruction.hpp"


class Decoder {
public:
  static Instruction Decode(const u8* mem);
};
