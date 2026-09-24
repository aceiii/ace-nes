#pragma once

#include "cpu.hpp"
#include "types.hpp"


namespace address {
  inline u16 IndexedZeroPageX(Cpu cpu, u16 addr) {
    cpu.Tick();
    return (addr + cpu.registers.x) & 0xFF;
  }

  inline u16 IndexedZeroPageY(Cpu& cpu, u16 addr) {
    cpu.Tick();
    return (addr + cpu.registers.y) & 0xFF;
  }

  inline u16 IndexedAbsoluteX(Cpu& cpu, u16 addr, bool force_tick = false) {
    u8 base = addr >> 8;
    u16 result = addr + cpu.registers.x;
    if (force_tick || base != ((result >> 8) & 0xFF)) {
      cpu.Tick();
    }
    return result;
  }

  inline u16 IndexedAbsoluteY(Cpu& cpu, u16 addr, bool force_tick = false) {
    u8 base = addr >> 8;
    u16 result = addr + cpu.registers.y;
    if (force_tick || base != ((result >> 8) & 0xFF)) {
      cpu.Tick();
    }
    return result;
  }
};
