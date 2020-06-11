#pragma once

#include <cstdint>

namespace CYD
{
// TODO Make these available at the game layer. Should be able to create custom components

/*
Here is a list of all available component types.
Some of these components are base components and others are derived components. For example,
RENDERABLE is a base type of all the other renderable component types. It is useful to make this
difference since we can then tell a system to take care of all components of the same BASE_TYPE but
still have different component pools based on the TYPE.
*/

// All entity component types.
enum class ComponentType : int16_t
{
   UNKNOWN = -1, // For unknown/undefined subtypes

   // Scene
   // ==============================================================================================
   TRANSFORM,
   CAMERA,

   // Lighting
   // ==============================================================================================
   LIGHT,              // Default, base type
   LIGHT_DIRECTIONAL,  // Subtypes
   LIGHT_POINT,
   LIGHT_SPOTLIGHT,  // Not yet implemented

   // Rendering
   // ==============================================================================================
   MESH,
   RENDERABLE,         // Default, base type
   RENDERABLE_CUSTOM,  // Subtypes
   RENDERABLE_PHONG,
   RENDERABLE_PBR,
   RENDERABLE_SKYBOX,

   // Procedural
   // ==============================================================================================
   OCEAN,

   // Physics
   // ==============================================================================================
   MOTION,

   // Behaviour
   // ==============================================================================================
   ENTITY_FOLLOW,

   COUNT  // Keep at the end
};
}
