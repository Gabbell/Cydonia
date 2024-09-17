#include <ECS/Systems/Debug/DebugDrawSystem.h>

#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MeshCache.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/Utility/Transforms.h>

#include <ECS/EntityManager.h>
#include <ECS/Entity.h>
#include <ECS/Components/Scene/ViewComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
#if CYD_DEBUG
static bool s_initialized = false;

static void Initialize() { s_initialized = true; }

static void DrawDebugSphere(
    CmdListHandle cmdList,
    const MeshCache& meshes,
    const TransformComponent& transform,
    const DebugDrawComponent::SphereParams& sphereParams )
{
   const PipelineIndex pipIdx = StaticPipelines::FindByName( "DEBUG" );
   const MeshIndex sphereMesh = meshes.findMesh( "DEBUG_SPHERE" );
   if( sphereMesh == INVALID_MESH_IDX || pipIdx == INVALID_PIPELINE_IDX )
   {
      return;
   }

   GRIS::BindPipeline( cmdList, pipIdx );

   const glm::mat4 modelMatrix = Transform::GetModelMatrix(
       glm::vec3( sphereParams.radius ), glm::quat(), transform.position );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

   meshes.bind( cmdList, sphereMesh );

   const MeshCache::DrawInfo drawInfo = meshes.getDrawInfo( sphereMesh );
   GRIS::DrawIndexed( cmdList, drawInfo.indexCount );
}
#endif

void DebugDrawSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

#if CYD_DEBUG

   if( !s_initialized )
   {
      Initialize();
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::DEBUG_DRAW );
   CYD_SCOPED_GPUTRACE( cmdList, "DebugDrawSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   GRIS::BeginRendering( cmdList, scene.mainFramebuffer );

   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   GRIS::BindUniformBuffer( cmdList, scene.viewsBuffer, 0 );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );
      const DebugDrawComponent& debug     = GetComponent<DebugDrawComponent>( entityEntry );

      const UploadToBufferInfo info = { 0, sizeof( debug.shaderParams ) };
      GRIS::UploadToBuffer( scene.debugParamsBuffer, &debug.shaderParams, info );

      GRIS::BindUniformBuffer( cmdList, scene.debugParamsBuffer, 1 );

      switch( debug.type )
      {
         case DebugDrawComponent::Type::SPHERE:
            DrawDebugSphere( cmdList, m_meshes, transform, debug.params.sphere );
            break;
         case DebugDrawComponent::Type::FRUSTUM:
         {
            CYD_ASSERT( !"Unimplemented" );
            break;
         }
      }
   }

   GRIS::EndRendering( cmdList );
#endif
}
}
