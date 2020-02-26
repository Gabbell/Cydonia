#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Handles/Handle.h>

#include <vector>
#include <string>

namespace cyd
{
struct Vertex;

class RenderableComponent final : public BaseComponent
{
  public:
   RenderableComponent() = default;                             // Nothing constructor
   RenderableComponent( const std::vector<Vertex>& vertices );  // Vertex constructor
   RenderableComponent( const std::string& meshPath );          // Mesh constructor
   COPIABLE( RenderableComponent )
   virtual ~RenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   bool init() override { return true; }
   void uninit() override;

   VertexBufferHandle vertexBuffer;  // Mesh data
   IndexBufferHandle indexBuffer;
   uint32_t indexCount;
   uint32_t vertexCount;

   UniformBufferHandle matBuffer;  // Material properties
   TextureHandle matTexture;
};
}
