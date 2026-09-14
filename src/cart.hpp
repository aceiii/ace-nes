#pragma once

#include <magic_enum/magic_enum.hpp>

#include "file.hpp"


enum class MirrorMode : u8 {
  Horizontal,
  Vertical,
};

enum class ConsoleType : u8 {
  Regular,
  VsStytem,
};

enum class VideoFormat : u8 {
  NTSC,
  PAL,
};

struct CartHeader {
  static constexpr std::array<u8, 4> kNesIdentifier = { 'N', 'E', 'S', 0x1A };

  std::array<u8, 4> ident; // bit 0-3
  u8 prg_rom_banks; // bit 4
  u8 chr_rom_banks; // bit 5

  struct { // bit 6
    u8 mirroring: 1;
    u8 battery : 1;
    u8 trainer : 1;
    u8 four_screen_vram : 1;
    u8 mapper_nybble_lo : 4;
  };

  struct { // bit 7
    u8 console_type : 1;
    u8 : 3;
    u8 mapper_nybble_hi : 4;
  };

  u8 prg_ram_banks; // bit 8

  struct { // bit 9
    u8 video_format : 1;
    u8 : 7;
  };

  std::array<u8, 6> reserved; // bit 10-15

  auto IsNesRom() const {
    return ident == CartHeader::kNesIdentifier;
  }

  auto GetMirrorMode() const {
    return static_cast<MirrorMode>(mirroring);
  }

  bool HasBattery() const {
    return battery;
  }

  bool HasTrainer() const {
    return trainer;
  }

  ConsoleType GetConsoleType() const {
    return static_cast<ConsoleType>(console_type);
  }

  VideoFormat GetVideoFormat() const {
    return static_cast<VideoFormat>(video_format);
  }
};

class Cart {
public:
  bool Load(std::string_view path);
  void Unload();
  bool IsLoaded() const;
  const CartHeader& Header() const;
  const Buffer& Rom() const;
  size_t Size() const;

private:
  bool loaded_;

  CartHeader header_;
  Buffer trainer_;
  Buffer bytes_;
};
