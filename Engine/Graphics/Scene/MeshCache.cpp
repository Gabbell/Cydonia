#include <Graphics/Scene/MeshCache.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/VertexLayout.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/GraphicsIO.h>
#include <Graphics/Utility/MeshGeneration.h>

namespace CYD
{
// Asset Paths Constants
// ================================================================================================
static constexpr char MESH_PATH[] = "../Engine/Data/Meshes/";

MeshCache::MeshCache()
{
   // Initialize default meshes
   _initDefaultMeshes();
}

void MeshCache::_initDefaultMeshes()
{
   CmdListHandle transferList = GRIS::CreateCommandList( QueueUsage::TRANSFER, "Init Default Meshes" );

//#if CYD_DEBUG
   std::vector<Vertex> verts;
   std::vector<uint32_t> indices;
   MeshGeneration::Icosphere( verts, indices, 2 );
   loadMesh( transferList, "DEBUG_SPHERE", verts, indices );
//#endif

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}

const Mesh& MeshCache::getMesh( const std::string_view name )
{
   const std::string meshString( name );
   auto it = m_meshes.find( meshString );
   if( it != m_meshes.end() )
   {
      return it->second;
   }

   return m_meshes[""];
}

bool MeshCache::loadMeshFromPath( CmdListHandle transferList, const std::string_view meshPath )
{
   const std::string meshString( meshPath );
   auto it = m_meshes.find( meshString );
   if( it == m_meshes.end() )
   {
      // Mesh was not previously loaded, load it
      const std::string fullPath = MESH_PATH + meshString + "/";

      std::vector<Vertex> vertices;
      std::vector<uint32_t> indices;
      GraphicsIO::LoadMesh( meshString, vertices, indices );

      Mesh& mesh = m_meshes[meshString];

      mesh.vertexBuffer = GRIS::CreateVertexBuffer(
          transferList,
          static_cast<uint32_t>( vertices.size() ),
          static_cast<uint32_t>( sizeof( Vertex ) ),
          vertices.data(),
          meshPath );

      mesh.vertexCount = static_cast<uint32_t>( vertices.size() );

      mesh.indexBuffer = GRIS::CreateIndexBuffer(
          transferList, static_cast<uint32_t>( indices.size() ), indices.data(), meshPath );

      mesh.indexCount = static_cast<uint32_t>( indices.size() );

      return true;
   }

   return false;
}

bool MeshCache::loadMesh(
    CmdListHandle transferList,
    const std::string_view name,
    const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices )
{
   const std::string meshString( name );
   auto it = m_meshes.find( meshString );
   if( it == m_meshes.end() )
   {
      Mesh& mesh = m_meshes[meshString];

      mesh.vertexBuffer = GRIS::CreateVertexBuffer(
          transferList,
          static_cast<uint32_t>( vertices.size() ),
          static_cast<uint32_t>( sizeof( Vertex ) ),
          vertices.data(),
          name );

      mesh.vertexCount = static_cast<uint32_t>( vertices.size() );

      mesh.indexBuffer = GRIS::CreateIndexBuffer(
          transferList, static_cast<uint32_t>( indices.size() ), indices.data(), name );

      mesh.indexCount = static_cast<uint32_t>( indices.size() );

      return true;
   }

   return false;
}
}