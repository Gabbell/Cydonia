#pragma once

#include <cstdint>

#include <Common/Include.h>

#include <string_view>

namespace CYD
{
// TODO Make these available at the game layer. Should be able to create custom components

// All entity component types.
enum class ComponentType : int16_t
{
   UNKNOWN = -1,  // For unknown/undefined subtypes

   // Scene
   // ==============================================================================================
   TRANSFORM,
   VIEW,

   // Lighting
   // ==============================================================================================
   LIGHT,
   SHADOW_MAP,

   // Rendering
   // ==============================================================================================
   MATERIAL,
   MESH,
   RENDERABLE,
   POST_PROCESS_VOLUME,  // TODO
   PARTICLES_VOLUME,     // TODO
   INSTANCED,
   TESSELLATED,

   // Procedural
   // ==============================================================================================
   DISPLACEMENT,
   OCEAN,
   FOG,
   ATMOSPHERE,

   // Physics
   // ==============================================================================================
   MOTION,

   // Behaviour
   // ==============================================================================================
   ENTITY_FOLLOW,

   // Debug
   // ==============================================================================================
   DEBUG_DRAW,

   COUNT  // Keep at the end
};

std::string_view GetComponentName( ComponentType type );
}
