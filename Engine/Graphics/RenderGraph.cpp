#include <Graphics/RenderGraph.h>

#include <Common/Assert.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/Utility/GraphicsIO.h>

namespace CYD
{
RenderGraph::RenderGraph()
{
   m_meshes.reserve( INITIAL_AMOUNT_RESOURCES );
   m_materials.reserve( INITIAL_AMOUNT_RESOURCES );
   m_buffers.reserve( INITIAL_AMOUNT_RESOURCES );

   m_viewBuffer  = GRIS::CreateUniformBuffer( sizeof( View ) );
   m_lightBuffer = GRIS::CreateUniformBuffer( sizeof( Light ) );
}

RenderGraph::~RenderGraph()
{
   // Go through all handles and destroy them

   GRIS::DestroyBuffer( m_lightBuffer );
   GRIS::DestroyBuffer( m_viewBuffer );
}

void RenderGraph::add3DRenderable(
    const glm::mat4& modelMatrix,
    StaticPipelines::Type pipType,
    const std::string_view materialPath,
    const std::string_view meshPath )
{
   const uint32_t pipIdx = static_cast<uint32_t>( pipType );

   Renderable3D& renderable = m_renderables[pipIdx][m_renderableCounts[pipIdx]++];
   renderable.modelMatrix   = modelMatrix;
   renderable.materialPath  = materialPath;
   renderable.meshPath      = meshPath;
}

void RenderGraph::addView(
    const std::string_view name,
    const glm::vec4& position,
    const glm::mat4& view,
    const glm::mat4& proj )
{
   auto it = m_views.find( name );
   if( it != m_views.end() )
   {
      CYDASSERT( !"RenderGraph: Overwriting an already existing view. Forgot to reset?" );
      return;
   }

   View& newView            = m_views[name];
   newView.position         = position;
   newView.viewMatrix       = view;
   newView.projectionMatrix = proj;
}

void RenderGraph::addLight(
    const glm::vec4& enabled,
    const glm::vec4& direction,
    const glm::vec4& color )
{
   m_light.enabled   = enabled;
   m_light.direction = direction;
   m_light.color     = color;
}

void RenderGraph::setViewport( float offsetX, float offsetY, float width, float height )
{
   m_viewport = Viewport{offsetX, offsetY, width, height};  // ...Default minDepth and maxDepth
}

void RenderGraph::setScissor( int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height )
{
   m_scissor = Rectangle{offsetX, offsetY, width, height};
}

void RenderGraph::reset()
{
   NodeGraph::reset();

   m_renderableCounts = {};

   m_views.clear();
}

bool RenderGraph::compile()
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   bool neededTransfer = false;

   for( uint32_t pipIdx = 0; pipIdx < (uint32_t)StaticPipelines::Type::COUNT; ++pipIdx )
   {
      const uint32_t renderableCount = m_renderableCounts[pipIdx];

      for( uint32_t i = 0; i < renderableCount; ++i )
      {
         const Renderable3D& renderable = m_renderables[pipIdx][i];

         neededTransfer |= _loadMesh( transferList, renderable.meshPath );
         neededTransfer |= _loadMaterial( transferList, pipIdx, renderable.materialPath );
      }
   }

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );

   GRIS::DestroyCommandList( transferList );

   return true;
}

bool RenderGraph::execute() const
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, true );

   // Get main view
   const auto viewIt = m_views.find( MAIN_VIEW_STRING );
   if( viewIt == m_views.end() )
   {
      CYDASSERT( !"RenderGraph: Could not find main view" );
   }

   // Copying view camera to view/projection UBO and binding it
   const View& mainView = viewIt->second;
   GRIS::CopyToBuffer( m_viewBuffer, &mainView, 0, sizeof( View ) );
   GRIS::BindUniformBuffer( cmdList, m_viewBuffer, 0, 0 );

   // Copying light data to light UBO and binding it
   GRIS::CopyToBuffer( m_lightBuffer, &m_light, 0, sizeof( Light ) );
   GRIS::BindUniformBuffer( cmdList, m_lightBuffer, 0, 1 );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, m_viewport );
   GRIS::SetScissor( cmdList, m_scissor );

   // Begin rendering straight to the swapchain
   GRIS::BeginRenderPassSwapchain( cmdList, true );

   for( uint32_t pipIdx = 0; pipIdx < (uint32_t)StaticPipelines::Type::COUNT; ++pipIdx )
   {
      const uint32_t renderableCount = m_renderableCounts[pipIdx];

      if( renderableCount == 0 )
      {
         continue;
      }

      const StaticPipelines::Type pipType = static_cast<StaticPipelines::Type>( pipIdx );
      GRIS::BindPipeline( cmdList, pipType );

      for( uint32_t i = 0; i < renderableCount; ++i )
      {
         const Renderable3D& renderable = m_renderables[pipIdx][i];

         // Prepare rendering
         GRIS::UpdateConstantBuffer(
             cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( glm::mat4 ), &renderable.modelMatrix );

         // Bind material
         auto materialIt = m_materials.find( renderable.materialPath );
         if( materialIt != m_materials.end() )
         {
            const Material& material = materialIt->second;

            if( material.albedo ) GRIS::BindTexture( cmdList, material.albedo, 1, 0 );
            if( material.normal ) GRIS::BindTexture( cmdList, material.normal, 1, 1 );
            if( material.metalness ) GRIS::BindTexture( cmdList, material.metalness, 1, 2 );
            if( material.roughness ) GRIS::BindTexture( cmdList, material.roughness, 1, 3 );

            // Optional
            if( material.ao ) GRIS::BindTexture( cmdList, material.ao, 1, 4 );
            if( material.height ) GRIS::BindTexture( cmdList, material.height, 1, 5 );
         }

         // Draw mesh
         auto meshIt = m_meshes.find( renderable.meshPath );
         if( meshIt != m_meshes.end() )
         {
            const Mesh& mesh = meshIt->second;

            GRIS::BindVertexBuffer( cmdList, mesh.vertexBuffer );

            if( mesh.indexBuffer )
            {
               // This renderable has an index buffer, use it to draw
               GRIS::BindIndexBuffer<uint32_t>( cmdList, mesh.indexBuffer );
               GRIS::DrawVerticesIndexed( cmdList, mesh.indexCount );
            }
            else
            {
               GRIS::DrawVertices( cmdList, mesh.vertexCount );
            }
         }
      }
   }

   GRIS::EndRenderPass( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
   GRIS::DestroyCommandList( cmdList );

   GRIS::RenderBackendCleanup();

   return true;
}

bool RenderGraph::_loadMesh( CmdListHandle transferList, const std::string_view meshPath )
{
   if( meshPath.empty() )
   {
      return false;
   }

   auto it = m_meshes.find( meshPath );
   if( it == m_meshes.end() )
   {
      // Mesh was not previously loaded, load it
      std::vector<Vertex> vertices;
      std::vector<uint32_t> indices;
      GraphicsIO::LoadMesh( std::string( meshPath ), vertices, indices );

      Mesh& mesh = m_meshes[meshPath];

      mesh.vertexBuffer = GRIS::CreateVertexBuffer(
          transferList,
          static_cast<uint32_t>( vertices.size() ),
          static_cast<uint32_t>( sizeof( Vertex ) ),
          vertices.data() );

      mesh.vertexCount = static_cast<uint32_t>( vertices.size() );

      mesh.indexBuffer = GRIS::CreateIndexBuffer(
          transferList, static_cast<uint32_t>( indices.size() ), indices.data() );

      mesh.indexCount = static_cast<uint32_t>( indices.size() );

      return true;
   }

   return false;
}

bool RenderGraph::_loadMaterial(
    CmdListHandle transferList,
    uint32_t /*pipType*/,
    std::string_view materialPath )
{
   if( materialPath.empty() )
   {
      return false;
   }

   auto it = m_materials.find( materialPath );
   if( it == m_materials.end() )
   {
      TextureDescription texDesc = {};
      texDesc.width              = 2048;
      texDesc.height             = 2048;
      texDesc.size               = texDesc.width * texDesc.height * sizeof( uint32_t );
      texDesc.type               = ImageType::TEXTURE_2D;
      texDesc.format             = PixelFormat::RGBA8_SRGB;
      texDesc.usage              = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
      texDesc.stages             = ShaderStage::FRAGMENT_STAGE;

      // TODO More dynamic resource loading. Maybe depending on the pipeline, load only what we need
      const std::string fullPath = "Data/Materials/" + std::string( materialPath ) + "/";

      const std::string albedoPath    = fullPath + "albedo.png";
      const std::string normalPath    = fullPath + "normal.png";
      const std::string metalnessPath = fullPath + "metalness.png";
      const std::string roughnessPath = fullPath + "roughness.png";
      const std::string aoPath        = fullPath + "ao.png";
      const std::string heightPath    = fullPath + "height.png";

      Material& material = m_materials[materialPath];

      material.albedo    = GRIS::CreateTexture( transferList, texDesc, albedoPath );
      material.normal    = GRIS::CreateTexture( transferList, texDesc, normalPath );
      material.height    = GRIS::CreateTexture( transferList, texDesc, heightPath );
      material.metalness = GRIS::CreateTexture( transferList, texDesc, metalnessPath );
      material.roughness = GRIS::CreateTexture( transferList, texDesc, roughnessPath );
      material.ao        = GRIS::CreateTexture( transferList, texDesc, aoPath );

      return true;
   }

   return false;
}

// State machine used to transfer in between states and detect state anomalies
void RenderGraph::_updateState( State desiredState )
{
   // Assume worst case
   State newState = State::ERROR;

   switch( m_currentState )
   {
      case State::INITIAL:
         if( desiredState == State::SETUP )
         {
            // The graph is brand new and we have just added our first node
            newState = State::SETUP;
         }
         break;
      default:
         newState = State::ERROR;
   }

   CYDASSERT(
       ( newState != State::ERROR ) && "RenderGraph: Encountered an error while updating state" );

   m_currentState = newState;
}
}
