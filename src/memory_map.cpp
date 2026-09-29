#include "memory_map.hpp"


u8 MemoryMap::Read(u16 address, BusMode mode) {
  if (address < 0x2000) {
    ram->Read(address, mode);
  }

  if (address < 0x4000) {
    ppu->Read(address, mode);
  }

  if (address < 0x4018) {
    return apu->Read(address, mode);
  }

  if (address < 0x4020) {
    // NOTE: unused
    return 0x00;
  }

  return cart->Read(address, mode);
}

void MemoryMap::Write(u16 address, u8 value, BusMode mode) {
  if (address < 0x2000) {
    return ram->Write(address, value, mode);
  }

  if (address < 0x4000) {
    return ppu->Write(address, value, mode);
  }

  if (address < 0x4018) {
    return apu->Write(address, value, mode);
  }

  if (address < 0x4020) {
    // NOTE: unused
    return;
  }

  return cart->Write(address, value, mode);
}
