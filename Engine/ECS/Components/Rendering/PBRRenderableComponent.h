#pragma once

#include <ECS/Components/Rendering/RenderableComponent.h>

#include <Handles/Handle.h>

#include <string>

namespace CYD
{
class PBRRenderableComponent final : public RenderableComponent
{
  public:
   PBRRenderableComponent();
   explicit PBRRenderableComponent( const std::string& assetName );
   COPIABLE( PBRRenderableComponent )
   virtual ~PBRRenderableComponent();

   bool init( const std::string& modelName );
   void uninit() override;

   TextureHandle albedo;
   TextureHandle normalMap;
   TextureHandle metalnessMap;
   TextureHandle roughnessMap;
   TextureHandle ambientOcclusionMap;
   TextureHandle heightMap;
};
}
