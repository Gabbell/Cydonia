#pragma once

#include <cstdint>

namespace cyd
{
// TODO Make these available at the game layer. Should be able to create custom components

// All entity component types.
enum class ComponentType : uint8_t
{
   TRANSFORM = 0,
   RENDERABLE,
   MOTION,
   CAMERA,
   CONTROLLER,

   COUNT  // Keep at the end
};
}
