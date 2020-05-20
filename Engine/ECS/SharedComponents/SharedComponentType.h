#pragma once

#include <cstdint>

namespace CYD
{
// TODO Make these available at the game layer. Should be able to create custom components

// All singleton component types (shared across systems)
enum class SharedComponentType : uint8_t
{
   INPUT = 0,
   CAMERA,
   SCENE,
   COUNT  //  Keep at the end
};
}