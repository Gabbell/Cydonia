#include <Graphics/Scene/MeshCache.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/GraphicsIO.h>
#include <Graphics/Utility/MeshGeneration.h>

#include <Multithreading/ThreadPool.h>

#include <Profiling.h>

namespace CYD
{
// Asset Paths Constants
// ================================================================================================
static constexpr char MESH_PATH[] = "../../Engine/Data/Meshes/";
static VertexLayout s_defaultLayout;

MeshCache::MeshCache( EMP::ThreadPool& threadPool ) : m_threadPool( threadPool )
{
   // Create default vertex layout
   s_defaultLayout.addAttribute( VertexLayout::Attribute::POSITION, PixelFormat::RGB32F );
   s_defaultLayout.addAttribute( VertexLayout::Attribute::TEXCOORD, PixelFormat::RGB32F );
}

MeshCache::Mesh::~Mesh()
{
   GRIS::DestroyVertexBuffer( vertexBuffer );
   GRIS::DestroyIndexBuffer( indexBuffer );
}

MeshIndex MeshCache::findMesh( std::string_view name ) const
{
   const std::string nameString( name );

   const auto& findIt = m_meshNames.find( nameString );
   if( findIt != m_meshNames.end() )
   {
      return findIt->second;
   }

   return INVALID_MESH_IDX;
}

MeshIndex MeshCache::addMesh( std::string_view name, const VertexLayout& layout )
{
   const std::string nameString( name );

   CYD_ASSERT( m_meshNames.find( nameString ) == m_meshNames.end() );

   const size_t newIdx     = m_meshes.insertObject( MESH_PATH + nameString, layout );
   m_meshNames[nameString] = newIdx;

   return newIdx;
}

bool MeshCache::meshReady( MeshIndex meshIdx ) const
{
   if( meshIdx == INVALID_MESH_IDX )
   {
      return false;
   }

   return m_meshes[meshIdx]->currentState == State::LOADED_TO_VRAM;
}

MeshCache::DrawInfo MeshCache::getDrawInfo( MeshIndex meshIdx ) const
{
   return { m_meshes[meshIdx]->vertexCount, m_meshes[meshIdx]->indexCount };
}

void MeshCache::_loadToRAM( Mesh& mesh )
{
   CYD_TRACE();

   CYD_ASSERT_AND_RETURN( mesh.currentState == State::LOADING_TO_RAM, return; );

   if( mesh.generator )
   {
      mesh.generator();
   }
   else
   {
      CYD_ASSERT( !mesh.path.empty() && "MeshCache: Mesh to load did not have a path" );
      GraphicsIO::LoadMesh( mesh.path, mesh.vertexList, mesh.indices );
   }

   mesh.currentState = State::LOADED_TO_RAM;
}

void MeshCache::_loadToVRAM( CmdListHandle cmdList, Mesh& mesh )
{
   CYD_TRACE();

   CYD_ASSERT_AND_RETURN( mesh.currentState == State::LOADING_TO_VRAM, return; );

   mesh.vertexBuffer = GRIS::CreateVertexBuffer( mesh.vertexList.getSize(), mesh.path );
   mesh.indexBuffer =
       GRIS::CreateIndexBuffer( mesh.indices.size() * sizeof( uint32_t ), mesh.path );

   GRIS::UploadToVertexBuffer( cmdList, mesh.vertexBuffer, mesh.vertexList );

   UploadToBufferInfo info = { 0, sizeof( uint32_t ) * mesh.indices.size() };
   GRIS::UploadToIndexBuffer( cmdList, mesh.indexBuffer, mesh.indices.data(), info );

   mesh.vertexCount = mesh.vertexList.getVertexCount();
   mesh.indexCount  = static_cast<uint32_t>( mesh.indices.size() );

   mesh.currentState = State::LOADED_TO_VRAM;
}

void MeshCache::bind( CmdListHandle cmdList, MeshIndex meshIdx ) const
{
   const Mesh* pMesh = m_meshes[meshIdx];
   CYD_ASSERT( pMesh );

   const Mesh& mesh = *pMesh;

   GRIS::BindVertexBuffer( cmdList, mesh.vertexBuffer );
   GRIS::BindIndexBuffer<uint32_t>( cmdList, mesh.indexBuffer );
}

MeshCache::State MeshCache::progressLoad( CmdListHandle cmdList, MeshIndex meshIdx )
{
   Mesh* pMesh = m_meshes[meshIdx];
   CYD_ASSERT( pMesh );

   Mesh& mesh = *pMesh;

   if( mesh.currentState == State::UNINITIALIZED )
   {
      // This material is not loaded, start a load job
      mesh.currentState = State::LOADING_TO_RAM;

      m_threadPool.submit( MeshCache::_loadToRAM, mesh );
   }

   if( mesh.currentState == State::LOADED_TO_RAM )
   {
      mesh.currentState = State::LOADING_TO_VRAM;

      _loadToVRAM( cmdList, mesh );
   }

   // We are setting the LOADED_TO_VRAM state right after loadToVRAM is called
   // This only works because we guarantee that the LOAD command list is first in the
   // render graph so the transfer command list will have  uploaded this mesh data
   // when the time comes to render. We can also free the data here because it has been uploaded
   // to a staging buffer already
   // TODO Make it so that we can right away upload to a staging buffer instead of having to copy
   if( mesh.currentState == State::LOADED_TO_VRAM )
   {
      // Make sure we're freeing RAM
      mesh.vertexList.freeList();
      mesh.indices.clear();
      mesh.indices.shrink_to_fit();
   }

   return mesh.currentState;
}
}