#pragma once

#include <cstdint>

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

static const char* GetSharedComponentName( SharedComponentType type )
{
   static constexpr char SHARED_COMPONENT_NAMES[][32] = { "Input", "Scene" };

   static_assert(
       ARRSIZE( SHARED_COMPONENT_NAMES ) == static_cast<size_t>( SharedComponentType::COUNT ) );

   return SHARED_COMPONENT_NAMES[static_cast<size_t>( type )];
}
}