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
   RenderableComponent() = default;

   // For Phong renderables with vertex data
   RenderableComponent( const std::vector<Vertex>& vertices );

   // For Phong renderables with a persistent mesh
   RenderableComponent( const std::string& meshPath );

   // For PBR renderables
   RenderableComponent( const std::string& meshPath, const std::string& pbrPath );

   COPIABLE( RenderableComponent );

   virtual ~RenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   bool init() override { return true; }
   void uninit() override;

   // TODO Duplicate renderable data like mesh or textures should point to the same handle
   VertexBufferHandle vertexBuffer;
   IndexBufferHandle indexBuffer;
   uint32_t indexCount;
   uint32_t vertexCount;

   TextureHandle albedo;
   TextureHandle normalMap;
   TextureHandle metallicMap;
   TextureHandle roughnessMap;
   TextureHandle ambientOcclusionMap;
   TextureHandle heightMap;
};
}
