#pragma once

#include <cstdint>

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
   CAMERA,

   // Lighting
   // ==============================================================================================
   LIGHT,

   // Rendering
   // ==============================================================================================
   MATERIAL,
   MESH,
   RENDERABLE,
   FULLSCREEN,

   // Procedural
   // ==============================================================================================
   NOISE,
   OCEAN,
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
   DEBUG_SPHERE,

   COUNT  // Keep at the end
};
}
