#include <Graphics/RenderGraph.h>

#include <Common/Assert.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/Utility/GraphicsIO.h>

namespace CYD
{
RenderGraph::RenderGraph()
{
   m_viewBuffer  = GRIS::CreateUniformBuffer( sizeof( View ) );
   m_lightBuffer = GRIS::CreateUniformBuffer( sizeof( Light ) );
}

RenderGraph::~RenderGraph()
{
   // Go through all handles and destroy them
   GRIS::DestroyBuffer( m_lightBuffer );
   GRIS::DestroyBuffer( m_viewBuffer );
}

void RenderGraph::addPass(const PassInfo& info)
{
  
}

void RenderGraph::add3DRenderable(
    const glm::mat4& modelMatrix,
    const std::string_view pipName,
    const std::string_view materialPath,
    const std::string_view meshPath )
{
   Renderable3D& renderable = m_renderables[pipName][m_renderableCounts[pipName]++];
   renderable.modelMatrix   = modelMatrix;
   renderable.materialPath  = materialPath;
   renderable.meshPath      = meshPath;
}

void RenderGraph::addSceneView(
    const std::string_view name,
    const glm::vec4& position,
    const glm::mat4& view,
    const glm::mat4& proj )
{
   auto it = m_views.find( name );
   if( it != m_views.end() )
   {
      CYDASSERT( !"RenderGraph: Overwriting an already existing view. Forgot to reset scene?" );
      return;
   }

   View& newView            = m_views[name];
   newView.position         = position;
   newView.viewMatrix       = view;
   newView.projectionMatrix = proj;
}

void RenderGraph::addSceneLight(
    const glm::vec4& enabled,
    const glm::vec4& direction,
    const glm::vec4& color )
{
   m_light.enabled   = enabled;
   m_light.direction = direction;
   m_light.color     = color;
}

void RenderGraph::setViewport(
    float offsetX,
    float offsetY,
    float width,
    float height,
    bool flippedY )
{
   // ...Default minDepth and maxDepth
   if( flippedY )
   {
      m_viewport = Viewport{ offsetX, height, width, -height };
   }
   else
   {
      m_viewport = Viewport{ offsetX, offsetY, width, height };
   }
}

void RenderGraph::setScissor( int32_t offsetX, int32_t offsetY, uint32_t width, uint32_t height )
{
   m_scissor = Rectangle{ offsetX, offsetY, width, height };
}

void RenderGraph::resetScene()
{
   NodeGraph::reset();

   m_renderableCounts = {};
   m_views.clear();
}

bool RenderGraph::compile()
{
   bool success = true;

   for( const auto& renderableArray : m_renderables )
   {
      const std::string_view pipName = renderableArray.first;

      const uint32_t renderableCount = m_renderableCounts[pipName];

      for( uint32_t i = 0; i < renderableCount; ++i )
      {
         const Renderable3D& renderable = m_renderables[pipName][i];

         success &= m_assets.loadMesh( renderable.meshPath );
         success &= m_assets.loadMaterial( renderable.materialPath );
      }
   }

   return success;
}

bool RenderGraph::execute()
{
   GRIS::PrepareFrame();

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
   GRIS::BeginRendering( cmdList, true );

   for( const auto& renderableArray : m_renderables )
   {
      const std::string_view pipName = renderableArray.first;

      const uint32_t renderableCount = m_renderableCounts[pipName];

      if( renderableCount == 0 )
      {
         continue;
      }

      GRIS::BindPipeline( cmdList, pipName );

      for( uint32_t i = 0; i < renderableCount; ++i )
      {
         const Renderable3D& renderable = m_renderables[pipName][i];

         // Prepare rendering
         GRIS::UpdateConstantBuffer(
             cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( glm::mat4 ), &renderable.modelMatrix );

         // Bind material
         const Material& material = m_assets.getMaterial( renderable.materialPath );

         if( material.albedo ) GRIS::BindTexture( cmdList, material.albedo, 1, 0 );
         if( material.normal ) GRIS::BindTexture( cmdList, material.normal, 1, 1 );
         if( material.metalness ) GRIS::BindTexture( cmdList, material.metalness, 1, 2 );
         if( material.roughness ) GRIS::BindTexture( cmdList, material.roughness, 1, 3 );

         // Optional
         if( material.ao ) GRIS::BindTexture( cmdList, material.ao, 1, 4 );
         if( material.height ) GRIS::BindTexture( cmdList, material.height, 1, 5 );

         // Draw mesh
         const Mesh& mesh = m_assets.getMesh( renderable.meshPath );

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

   GRIS::EndRendering( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
   GRIS::DestroyCommandList( cmdList );

   GRIS::PresentFrame();
   GRIS::RenderBackendCleanup();

   return true;
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
