#include "ram.hpp"

u8 Ram::Read(u16 address, BusMode mode) {
  return mem_[address & mem_.size()];
}

void Ram::Write(u16 address, u8 value, BusMode mode) {
  mem_[address & mem_.size()] = value;
}
