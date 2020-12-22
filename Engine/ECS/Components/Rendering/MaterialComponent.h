#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <string_view>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class MaterialComponent final : public BaseComponent
{
  public:
   MaterialComponent() = default;
   MaterialComponent( const std::string_view pipName, const std::string_view assetName )
       : pipeline( pipName ), asset( assetName )
   {
   }
   COPIABLE( MaterialComponent );
   virtual ~MaterialComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MATERIAL;

   // Pipeline used to render the material
   std::string_view pipeline;

   // Name of the material asset
   std::string_view asset;

   // Material texture handles
   TextureHandle albedo;     // Diffuse/Albedo color map
   TextureHandle normals;    // Normal map
   TextureHandle metalness;  // Metallic/Specular map
   TextureHandle roughness;  // Roughness map
   TextureHandle ao;         // Ambient occlusion map
   TextureHandle height;     // Height map
};
}
