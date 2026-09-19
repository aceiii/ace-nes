#pragma once

#include <bit>
#include <span>

#include "types.hpp"


struct StatusReg {
  union {
    struct {
      u8 carry: 1;
      u8 zero: 1;
      u8 interrupt_disable: 1;
      u8 decimal: 1;
      u8 b_flag : 1;
      u8 ignored : 1;
      u8 overflow: 1;
      u8 negative: 1;
    };
    u8 val;
  };
};

struct Registers {
  u8 a;
  u8 x;
  u8 y;
  u8 sp;
  StatusReg p;
  u16 pc;
};

class Cpu {
public:
  Registers registers {};
  u64 cycles {};
  IBus* bus {};

  void Step();

  u8 ReadNext();
  u16 ReadNext16();

  u8 Read(u16 address);
  void Write(u16 address, u8 value);

  void Tick();
};
