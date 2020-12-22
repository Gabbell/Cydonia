#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/GraphicsTypes.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class SkyboxComponent final : public BaseComponent
{
  public:
   SkyboxComponent() = default;
   SkyboxComponent( const std::string_view assetName ) : asset( assetName ) {}
   COPIABLE( SkyboxComponent );
   virtual ~SkyboxComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::SKYBOX;

   std::string_view asset;

   TextureHandle cubeMap;
};
}
