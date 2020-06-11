#pragma once

#include <ECS/Components/Rendering/RenderableComponent.h>

#include <Handles/Handle.h>

#include <ECS/Components/ComponentTypes.h>

#include <string>

namespace CYD
{
class PBRRenderableComponent final : public RenderableComponent
{
  public:
   PBRRenderableComponent() = default;
   explicit PBRRenderableComponent( const std::string& assetName );
   COPIABLE( PBRRenderableComponent )
   virtual ~PBRRenderableComponent();

   bool init( const std::string& modelName );
   void uninit() override;

   ComponentType getType() const override { return SUBTYPE; }

   static constexpr ComponentType SUBTYPE = ComponentType::RENDERABLE_PBR;

   TextureHandle albedo;
   TextureHandle normalMap;
   TextureHandle metalnessMap;
   TextureHandle roughnessMap;
   TextureHandle ambientOcclusionMap;
   TextureHandle heightMap;
};
}
