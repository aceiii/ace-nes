#include <span>
#include <spdlog/spdlog.h>

#include "cart.hpp"


bool Cart::Load(std::string_view path) {
  if (auto res = file::LoadBytes(path); res.has_value()) {
    auto& rom_bytes = res.value();

    const auto header_size = sizeof(CartHeader);
    std::memcpy(&header_, rom_bytes.data(), header_size);

    auto rom_size = rom_bytes.size() - header_size;

    bytes_.clear();
    bytes_.reserve(rom_size);
    std::copy(rom_bytes.begin() + header_size, rom_bytes.end(), bytes_.begin());

    spdlog::trace("rom size: {}", rom_size);

    if (!header_.IsNesRom()) {
      Unload();
      return false;
    }

    loaded_ = true;
    return true;
  }

  return false;
}

void Cart::Unload() {
  header_ = {};
  bytes_.clear();
  loaded_ = false;
}

bool Cart::IsLoaded() const {
  return loaded_;
}

const CartHeader& Cart::Header() const {
  return header_;
}

const Buffer &Cart::Rom() const {
  return bytes_;
}

size_t Cart::Size() const {
    return bytes_.size();
}
