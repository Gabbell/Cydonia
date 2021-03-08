#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/Material.h>

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace CYD
{
class Vertex;

struct Mesh
{
   ~Mesh();
   VertexBufferHandle vertexBuffer;
   IndexBufferHandle indexBuffer;
   uint32_t vertexCount = 0;
   uint32_t indexCount  = 0;
};

// For read-only assets loaded from disk
class AssetStash final
{
  public:
   AssetStash() = default;
   NON_COPIABLE( AssetStash );
   virtual ~AssetStash() = default;

   // void cleanup();

   const Mesh& getMesh( const std::string_view name );
   const Material& getMaterial( const std::string_view name );

   bool loadMaterialFromPath( CmdListHandle transferList, const std::string_view materialPath );
   bool loadMeshFromPath( CmdListHandle transferList, const std::string_view meshPath );
   bool loadMesh(
       CmdListHandle transferList,
       const std::string_view name,
       const std::vector<Vertex>& vertices,
       const std::vector<uint32_t>& indices );

  private:
   static constexpr uint32_t INITIAL_AMOUNT_RESOURCES = 128;

   std::unordered_map<std::string, Material> m_materials;
   std::unordered_map<std::string, Mesh> m_meshes;
};
}
