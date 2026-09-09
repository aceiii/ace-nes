#pragma once

#include "instruction.hpp"


class Decoder {
public:
  static Instruction Decode(u8* mem);
};
