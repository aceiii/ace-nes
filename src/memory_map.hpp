#pragma once

#include <memory>

#include "bus.hpp"
#include "types.hpp"


class MemoryMap : public IBus {
public:
  std::shared_ptr<IBus> ram;
  std::shared_ptr<IBus> ppu;
  std::shared_ptr<IBus> apu;
  std::shared_ptr<IBus> cart;

  u8 Read(u16 address, BusMode mode = BusMode::Normal) override;
  void Write(u16 address, u8 value, BusMode mode = BusMode::Normal) override;
};
