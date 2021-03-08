#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <string_view>

namespace CYD
{
class Material final
{
  public:
   Material() = default;
   Material( const std::string_view pipName ) : pipeline( pipName ) {}
   MOVABLE( Material );
   ~Material();

   // Pipeline used to render the material
   std::string_view pipeline;

   static constexpr uint32_t MAX_RESOURCE_BINDING = 32;

   // Material texture handles
   TextureHandle albedo;     // Diffuse/Albedo color map
   TextureHandle normals;    // Normal map
   TextureHandle disp;       // Height/Displacement map
   TextureHandle metalness;  // Metallic/Specular map
   TextureHandle roughness;  // Roughness map
   TextureHandle ao;         // Ambient occlusion map
};
}