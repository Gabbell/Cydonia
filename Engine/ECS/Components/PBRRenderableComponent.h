#pragma once

#include <ECS/Components/RenderableComponent.h>

#include <Handles/Handle.h>

#include <vector>
#include <string>

namespace cyd
{
struct Vertex;

class PBRRenderableComponent final : public RenderableComponent
{
  public:
   PBRRenderableComponent() = default;
   explicit PBRRenderableComponent( const std::string& modelName );
   COPIABLE( PBRRenderableComponent )
   virtual ~PBRRenderableComponent() = default;

   bool init() override { return true; }
   void uninit() override;

   static constexpr RenderableType RENDERABLE_TYPE = RenderableType::PBR;

   TextureHandle albedo;
   TextureHandle normalMap;
   TextureHandle metalnessMap;
   TextureHandle roughnessMap;
   TextureHandle ambientOcclusionMap;
   TextureHandle heightMap;
};
}
