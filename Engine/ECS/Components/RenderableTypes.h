#pragma once

namespace cyd
{
enum class RenderableType : uint8_t
{
   UNKNOWN = 0,
   PHONG,
   PBR,

   COUNT  // Keep at the end
};
}