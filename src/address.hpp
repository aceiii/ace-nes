#pragma once

#include "cpu.hpp"
#include "types.hpp"


namespace address {
  inline u16 IndexedZeroPageX(Cpu* cpu, u16 addr) {
    return (addr + cpu->registers.x) & 0xFF;
  }

  inline u16 IndexedZeroPageY(Cpu* cpu, u16 addr) {
    return (addr + cpu->registers.y) & 0xFF;
  }

  inline u16 IndexedAbsoluteX(Cpu* cpu, u16 addr) {
    return addr + cpu->registers.x;
  }

  inline u16 IndexedAbsoluteY(Cpu* cpu, u16 addr) {
    return addr + cpu->registers.y;
  }
};
