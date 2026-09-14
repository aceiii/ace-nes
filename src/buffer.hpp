#pragma once

#include <array>
#include <vector>

#include "types.hpp"

using Buffer = std::vector<u8>;

template <size_t Size>
using StaticBuffer = std::array<u8, Size>;
