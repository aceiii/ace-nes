#pragma once

#include "types.hpp"


enum class BusMode {
  Normal,
  DMA,
};

class IBus {
public:
  virtual ~IBus() = default;

  virtual u8 Read(u16 address, BusMode mode = BusMode::Normal) = 0;
  virtual void Write(u16 address, u8 value, BusMode mode = BusMode::Normal) = 0;
};
