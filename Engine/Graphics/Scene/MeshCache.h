#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/Scene/Mesh.h>

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace CYD
{
class Vertex;

// For read-only assets loaded from disk
class MeshCache final
{
  public:
   MeshCache() = default;
   NON_COPIABLE( MeshCache );
   virtual ~MeshCache() = default;

   // void cleanup();

   const Mesh& getMesh( const std::string_view name );

   bool loadMeshFromPath( CmdListHandle transferList, const std::string_view meshPath );
   bool loadMesh(
       CmdListHandle transferList,
       const std::string_view name,
       const std::vector<Vertex>& vertices,
       const std::vector<uint32_t>& indices );

  private:
   static constexpr uint32_t INITIAL_AMOUNT_RESOURCES = 128;

   std::unordered_map<std::string, Mesh> m_meshes;
};
}
