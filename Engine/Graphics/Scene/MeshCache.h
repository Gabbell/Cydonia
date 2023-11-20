#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/GraphicsTypes.h>
#include <Graphics/Vertex.h>

#include <Common/ObjectPool.h>

#include <cstdint>
#include <mutex>
#include <string_view>
#include <unordered_map>

namespace EMP
{
class ThreadPool;
}

namespace CYD
{
// For read-only assets loaded from disk
class MeshCache final
{
  public:
   MeshCache( EMP::ThreadPool& threadPool );
   NON_COPIABLE( MeshCache );
   ~MeshCache() = default;

   enum class State
   {
      UNINITIALIZED,
      LOADING_TO_RAM,
      LOADED_TO_RAM,  // RAM Resident
      LOADING_TO_VRAM,
      LOADED_TO_VRAM  // VRAM Resident
   };

   struct DrawInfo
   {
      uint32_t vertexCount;
      uint32_t indexCount;
   };

   MeshIndex findMesh( std::string_view name ) const;
   MeshIndex addMesh( std::string_view name, const VertexLayout& layout );

   DrawInfo getDrawInfo( MeshIndex meshIdx ) const;

   State progressLoad( CmdListHandle cmdList, MeshIndex meshIdx );

   void bind( CmdListHandle cmdList, MeshIndex index ) const;

  private:
   //  ============================================================================================
   // Mesh Entry
   /*
    * This is a reference to a mesh
    */

   struct Mesh
   {
      Mesh() = default;
      Mesh( std::string_view path, const VertexLayout& layout ) : path( path )
      {
         vertices.setLayout( layout );
      }
      MOVABLE( Mesh );
      ~Mesh();

      VertexList vertices;
      std::vector<uint32_t> indices;  // TODO Turn into IndexList

      std::string path;

      VertexBufferHandle vertexBuffer;
      IndexBufferHandle indexBuffer;

      uint32_t vertexCount = 0;
      uint32_t indexCount  = 0;

      State currentState = State::UNINITIALIZED;
   };

   void _initDefaultMeshes();
   static void _loadToRAM( Mesh& mesh );
   static void _loadToVRAM( CmdListHandle transferList, Mesh& mesh );

   static constexpr uint32_t INITIAL_AMOUNT_RESOURCES = 128;

   EMP::ThreadPool& m_threadPool;
   EMP::ObjectPool<Mesh> m_meshes;
   std::unordered_map<std::string, MeshIndex> m_meshNames;
};
}
