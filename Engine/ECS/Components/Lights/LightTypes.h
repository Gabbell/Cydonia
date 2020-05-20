#pragma once

namespace CYD
{
enum class LightType : uint8_t
{
   UNKNOWN = 0,
   DIRECTIONAL,
   POINT,
   SPOTLIGHT,

   COUNT  // Keep at the end
};
}
