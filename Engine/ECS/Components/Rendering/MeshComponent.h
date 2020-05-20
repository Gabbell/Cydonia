#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Handles/Handle.h>

#include <ECS/Components/ComponentTypes.h>

#include <string>
#include <vector>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
struct Vertex;

class MeshComponent : public BaseComponent
{
  public:
   MeshComponent() = default;
   MOVABLE( MeshComponent )
   virtual ~MeshComponent();

   bool init( const std::vector<Vertex>& vertices );
   bool init( const std::string& meshPath );
   bool init( const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices );

   void uninit() override;

   static constexpr ComponentType TYPE = ComponentType::MESH;

   // TODO Duplicate renderable data like mesh or textures should point to the same handle
   VertexBufferHandle vertexBuffer;
   IndexBufferHandle indexBuffer;
   uint32_t indexCount  = 0;
   uint32_t vertexCount = 0;
};
}
