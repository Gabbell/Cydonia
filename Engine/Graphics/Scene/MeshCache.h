#pragma once

#include <Common/Include.h>
#include <Common/ObjectPool.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/VertexList.h>
#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/Utility/MeshGeneration.h>

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

   // Management
   // ============================================================================================
   MeshIndex findMesh( std::string_view name ) const;
   MeshIndex addMesh( std::string_view name, const VertexLayout& layout );

   template <typename MeshGenerationFunction, typename... Args>
   MeshIndex loadMesh(
       CmdListHandle cmdList,
       std::string_view name,
       const VertexLayout& layout,
       MeshGenerationFunction genFunc,
       Args&&... args )
   {
      const MeshIndex newMeshIdx = addMesh( name, layout );

      Mesh& newMesh = *m_meshes[newMeshIdx];
      newMesh.vertexList.setLayout( layout );
      newMesh.generator = std::bind(
          genFunc, std::ref( newMesh.vertexList ), std::ref( newMesh.indices ), args... );

      newMesh.currentState = State::LOADING_TO_RAM;
      _loadToRAM( newMesh );

      newMesh.currentState = State::LOADING_TO_VRAM;
      _loadToVRAM( cmdList, newMesh );

      return newMeshIdx;
   }

   template <typename MeshGenerationFunction, typename... Args>
   MeshIndex enqueueMesh(
       std::string_view name,
       const VertexLayout& layout,
       MeshGenerationFunction genFunc,
       Args&&... args )
   {
      const MeshIndex newMeshIdx = addMesh( name, layout );
      Mesh* newMesh              = m_meshes[newMeshIdx];
      newMesh->vertexList.setLayout( layout );
      newMesh->generator = std::bind(
          genFunc, std::ref( newMesh->vertexList ), std::ref( newMesh->indices ), args... );
      return newMeshIdx;
   }

   // Loading
   // ============================================================================================
   State progressLoad( CmdListHandle cmdList, MeshIndex meshIdx );

   // Binding
   // ============================================================================================
   void bind( CmdListHandle cmdList, MeshIndex index ) const;

   // Drawing
   // ============================================================================================
   bool meshReady( MeshIndex meshIdx ) const;
   DrawInfo getDrawInfo( MeshIndex meshIdx ) const;

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
         vertexList.setLayout( layout );
      }
      MOVABLE( Mesh );
      ~Mesh();

      VertexList vertexList;
      std::vector<uint32_t> indices;  // TODO Turn into IndexList

      std::string path;
      MeshGeneration::Func generator;

      VertexBufferHandle vertexBuffer;
      IndexBufferHandle indexBuffer;

      uint32_t vertexCount = 0;
      uint32_t indexCount  = 0;

      State currentState = State::UNINITIALIZED;
   };

   static void _loadToRAM( Mesh& mesh );
   static void _loadToVRAM( CmdListHandle transferList, Mesh& mesh );

   static constexpr uint32_t INITIAL_AMOUNT_RESOURCES = 128;

   EMP::ThreadPool& m_threadPool;
   EMP::ObjectPool<Mesh> m_meshes;
   std::unordered_map<std::string, MeshIndex> m_meshNames;
};
}
