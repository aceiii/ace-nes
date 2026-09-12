#include <span>

#include "cart.hpp"

bool Cart::Load(std::string_view path)
{
  if (auto res = file::LoadBytes(path); res.has_value()) {
    auto& rom_bytes = res.value();

    bytes_ = res.value();

    auto header_bytes = std::span<u8>(bytes_).subspan(0, sizeof(CartHeader));
    std::memcpy(&header_, header_bytes.data(), header_bytes.size());

    if (!header_.IsNesRom()) {
      Unload();
      return false;
    }

    loaded_ = true;
    return true;
  }

  return false;
}

void Cart::Unload()
{
  header_ = {};
  bytes_.clear();
  loaded_ = false;
}

bool Cart::IsLoaded() const
{
  return loaded_;
}

const CartHeader& Cart::Header() const
{
  return header_;
}

size_t Cart::Size() const
{
    return bytes_.size();
}
