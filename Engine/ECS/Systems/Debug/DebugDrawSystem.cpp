#include <ECS/Systems/Debug/DebugDrawSystem.h>

#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MeshCache.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/Utility/Transforms.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
void DebugDrawSystem::tick( double /*deltaS*/ )
{
#if CYD_DEBUG
   CYD_TRACE( "DebugDrawSystem" );

   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   const auto& it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "MAIN" );
   if( it == scene.viewNames.end() )
   {
      // TODO WARNING
      CYD_ASSERT( !"Could not find main view, skipping render tick" );
      return;
   }
   const uint32_t viewIdx = static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::DEBUG_DRAW );
   CYD_SCOPED_GPUTRACE( cmdList, "DebugDrawSystem" );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );

   // Rendering straight to swapchain
   GRIS::BeginRendering( cmdList );

   PipelineIndex pipIdx = StaticPipelines::FindByName( "DEBUG" );

   GRIS::BindPipeline( cmdList, pipIdx );

   GRIS::BindUniformBuffer(
       cmdList,
       scene.viewsBuffer,
       0,
       0,
       viewIdx * sizeof( SceneComponent::ViewShaderParams ),
       sizeof( SceneComponent::ViewShaderParams ) );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const DebugDrawComponent& debug     = *std::get<DebugDrawComponent*>( entityEntry.arch );

      glm::mat4 modelMatrix( 1.0f );

      GRIS::CopyToBuffer(
          scene.debugParamsBuffer, &debug.shaderParams, 0, sizeof( debug.shaderParams ) );

      switch( debug.type )
      {
         case DebugDrawComponent::Type::SPHERE:
         {
            const DebugDrawComponent::SphereParams& sphere = debug.params.sphere;

            modelMatrix =
                Transform::GetModelMatrix( transform.scaling, glm::quat(), transform.position );

            // Update model transform push constant
            GRIS::UpdateConstantBuffer(
                cmdList, PipelineStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

            GRIS::BindUniformBuffer( cmdList, scene.debugParamsBuffer, 1, 0 );

            const Mesh& sphereMesh = m_meshes.getMesh( "DEBUG_SPHERE" );
            GRIS::BindVertexBuffer<Vertex>( cmdList, sphereMesh.vertexBuffer );
            GRIS::BindIndexBuffer<uint32_t>( cmdList, sphereMesh.indexBuffer );
            GRIS::DrawIndexed( cmdList, sphereMesh.indexCount );
         }
         break;
         case DebugDrawComponent::Type::NORMALS:
         {
         }
         break;
         case DebugDrawComponent::Type::TEXTURE:
         {
         }
         break;
      }
   }

   GRIS::EndRendering( cmdList );
#endif
}
}
