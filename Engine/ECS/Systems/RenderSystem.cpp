#include <ECS/Systems/RenderSystem.h>

#include <Graphics/RenderInterface.h>

bool cyd::RenderSystem::init() { return true; }

void cyd::RenderSystem::tick( double deltaMs )
{
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, true );
   GRIS::StartRecordingCommandList( cmdList );

   for( const auto& tuple : m_archetypes )
   {
      const TransformComponent* transform   = std::get<TransformComponent*>( tuple );
      const RenderableComponent* renderable = std::get<RenderableComponent*>( tuple );

      MVP mvp;
      mvp.model = glm::translate( glm::mat4( 1.0f ), transform->position ) *
                  glm::scale( glm::mat4( 1.0f ), transform->scaling ) *
                  glm::toMat4( transform->rotation );
      mvp.view = glm::mat4( 1.0f );
      mvp.proj = glm::mat4( 1.0f );

      GRIS::MapUniformBufferMemory( renderable->uboBuffer, &mvp );

      GRIS::BindPipeline( cmdList, renderable->pipInfo );
      GRIS::SetViewport( cmdList, { { 0.0f, 0.0f }, renderable->extent } );
      GRIS::BindTexture( cmdList, renderable->texture );
      GRIS::BindVertexBuffer( cmdList, renderable->vertexBuffer );  // TODO Index buffer
      GRIS::BindUniformBuffer( cmdList, renderable->uboBuffer );
      GRIS::BeginRenderPass( cmdList );
      GRIS::DrawVertices( cmdList, 3 );  // TODO Generalize
      GRIS::EndRenderPass( cmdList );
   }

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
   GRIS::PresentFrame();

   GRIS::DestroyCommandList( cmdList );

   GRIS::RenderBackendCleanup();
}
