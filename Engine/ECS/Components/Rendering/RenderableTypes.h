#pragma once

#include <cstdint>

namespace CYD
{
enum class RenderableType : uint8_t
{
   DEFAULT = 0,
   CUSTOM,
   PHONG,
   PBR,

   COUNT  // Keep at the end
};
}