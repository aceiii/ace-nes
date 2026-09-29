#pragma once

#include "array"

#include "bus.hpp"
#include "types.hpp"


class Ram : public IBus {
public:
  u8 Read(u16 address, BusMode mode = BusMode::Normal) override;
  void Write(u16 address, u8 value, BusMode mode = BusMode::Normal) override;

private:
  std::array<u8, 2048> mem_;
};
