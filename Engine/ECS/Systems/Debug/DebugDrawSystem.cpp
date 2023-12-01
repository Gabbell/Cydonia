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
   CYD_TRACE();

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

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

   GRIS::BeginRendering( cmdList, scene.mainFramebuffer );

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
      const TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );
      const DebugDrawComponent& debug     = GetComponent<DebugDrawComponent>( entityEntry );

      glm::mat4 modelMatrix( 1.0f );

      const UploadToBufferInfo info = { 0, sizeof( debug.shaderParams ) };
      GRIS::UploadToBuffer( scene.debugParamsBuffer, &debug.shaderParams, info );

      switch( debug.type )
      {
         case DebugDrawComponent::Type::SPHERE:
         {
            const MeshIndex sphereMesh = m_meshes.findMesh( "DEBUG_SPHERE" );
            if( sphereMesh == INVALID_MESH_IDX )
            {
               continue;
            }

            const DebugDrawComponent::SphereParams& sphere = debug.params.sphere;
            const MeshCache::DrawInfo drawInfo             = m_meshes.getDrawInfo( sphereMesh );

            modelMatrix = Transform::GetModelMatrix(
                glm::vec3( sphere.radius ), glm::quat(), transform.position );

            // Update model transform push constant
            GRIS::UpdateConstantBuffer(
                cmdList, PipelineStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

            GRIS::BindUniformBuffer( cmdList, scene.debugParamsBuffer, 1, 0 );

            m_meshes.bind( cmdList, sphereMesh );
            GRIS::DrawIndexed( cmdList, drawInfo.indexCount );
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
