#pragma once

#include <cstdint>

#include <Common/Include.h>

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
   INSTANCED,
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
   PROCEDURAL_DISPLACEMENT,
   PROCEDURAL_MATERIAL,
   OCEAN,
   FOG,

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

static const char* GetComponentName( ComponentType type )
{
   static constexpr char COMPONENT_NAMES[][32] = {
       "Transform",
       "Instanced",
       "Camera",
       "Light",
       "Material",
       "Mesh",
       "Renderable",
       "Fullscreen",
       "Procedural Displacement",
       "Procedural Material",
       "Ocean",
       "Fog",
       "Motion",
       "Entity Follow",
       "Debug Draw" };

   static_assert( ARRSIZE( COMPONENT_NAMES ) == static_cast<size_t>( ComponentType::COUNT ) );

   return COMPONENT_NAMES[static_cast<size_t>( type )];
}
}
