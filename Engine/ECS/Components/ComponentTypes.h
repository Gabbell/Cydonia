#pragma once

#include <cstdint>

namespace CYD
{
// TODO Make these available at the game layer. Should be able to create custom components

// All entity component types.
enum class ComponentType : uint8_t
{
   // Scene
   // ==============================================================================================
   TRANSFORM = 0,
   CAMERA,
   CONTROLLER,
   MESH,
   RENDERABLE,
   LIGHT,
   OCEAN,

   // Physics
   // ==============================================================================================
   MOTION,

   COUNT  // Keep at the end
};
}
