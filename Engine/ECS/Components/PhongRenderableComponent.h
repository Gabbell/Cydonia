#pragma once

#include <ECS/Components/RenderableComponent.h>
#include <ECS/Components/RenderableTypes.h>

#include <Handles/Handle.h>

#include <vector>
#include <string>

namespace cyd
{
struct Vertex;

class PhongRenderableComponent final : public RenderableComponent
{
  public:
   PhongRenderableComponent() = default;
   explicit PhongRenderableComponent( const std::vector<Vertex>& vertices );
   explicit PhongRenderableComponent( const std::string& meshPath );
   COPIABLE( PhongRenderableComponent );
   virtual ~PhongRenderableComponent() = default;

   bool init() override { return true; }
   void uninit() override;

   TextureHandle texture;
};
}
