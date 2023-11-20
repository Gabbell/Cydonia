#pragma once

#include <Common/Include.h>

#include <cstdint>
#include <string_view>

namespace CYD
{
// TODO Make these available at the game layer. Should be able to create custom components

// All singleton component types (shared across systems)
enum class SharedComponentType : uint8_t
{
   INPUT,
   SCENE,
   COUNT  //  Keep at the end
};

std::string_view GetSharedComponentName( SharedComponentType type );
}