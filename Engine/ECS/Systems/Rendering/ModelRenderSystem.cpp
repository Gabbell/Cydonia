#include <ECS/Systems/Rendering/ModelRenderSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Scene/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
// ================================================================================================
bool ModelRenderSystem::_compareEntities( const EntityEntry& first, const EntityEntry& second )
{
   // We are sorting entities by their material/pipeline
   const StaticMaterialComponent& matFirst  = *std::get<StaticMaterialComponent*>( first.arch );
   const StaticMaterialComponent& matSecond = *std::get<StaticMaterialComponent*>( second.arch );

   return matFirst.materialIdx < matSecond.materialIdx;
}

// ================================================================================================
static void optionalBufferBinding(
    CmdListHandle cmdList,
    BufferHandle buffer,
    std::string_view name,
    const PipelineInfo* pipInfo,
    uint32_t offset = 0,
    uint32_t range  = 0 )
{
   if( FlatShaderBinding res = pipInfo->findBinding( buffer, name ); res.valid )
   {
      GRIS::BindUniformBuffer( cmdList, buffer, res.binding, res.set, offset, range );
   }
}

// ================================================================================================
void ModelRenderSystem::tick( double /*deltaS*/ )
{
   // Finding main view
   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   const auto& it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "MAIN" );
   if( it == scene.viewNames.end() )
   {
      // TODO WARNING
      CYDASSERT( !"Could not find main view, skipping render tick" );
      return;
   }
   const uint32_t viewIdx = static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );

   // Start command list recording
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, "ModelRenderSystem", true );

   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );

   // Rendering straight to swapchain
   GRIS::BeginRendering( cmdList );

   // Tracking
   std::string_view prevMesh;
   PipelineIndex prevPipeline = INVALID_PIPELINE_IDX;
   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
       // Read-only components
      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const StaticMaterialComponent& material   = *std::get<StaticMaterialComponent*>( entityEntry.arch );
      const MeshComponent& mesh           = *std::get<MeshComponent*>( entityEntry.arch );

      // Pipeline
      if( prevPipeline != material.pipelineIdx )
      {
         const PipelineInfo* pipInfo = StaticPipelines::Get( material.pipelineIdx );
         if( pipInfo == nullptr )
         {
            // TODO WARNINGS
            printf( "ModelRenderSystem: Passed a null pipeline, skipping entity" );
            continue;
         }

         GRIS::BindPipeline( cmdList, pipInfo );

         // Scene bindings
         optionalBufferBinding(
             cmdList,
             scene.viewsBuffer,
             "EnvironmentView",
             pipInfo,
             viewIdx * sizeof( SceneComponent::ViewUBO ),
             sizeof( SceneComponent::ViewUBO ) );
         optionalBufferBinding( cmdList, scene.lightsBuffer, "DirectionalLight", pipInfo );

         prevPipeline = material.pipelineIdx;
      }

      const glm::mat4 modelMatrix = glm::translate( glm::mat4( 1.0f ), transform.position ) *
                                    glm::scale( glm::mat4( 1.0f ), transform.scaling ) *
                                    glm::toMat4( transform.rotation );

      // Update model transform push constant
      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

      // Material
      if( prevMaterial != material.materialIdx )
      {
         m_materials.bind( cmdList, material.materialIdx, 1 /*set*/ );
         prevMaterial = material.materialIdx;
      }

      // Vertex and index buffers
      if( prevMesh != mesh.asset )
      {
         if( mesh.vertexBuffer )
         {
            GRIS::BindVertexBuffer<Vertex>( cmdList, mesh.vertexBuffer );
            if( mesh.indexBuffer )
            {
               // This renderable has an index buffer, use it to draw
               GRIS::BindIndexBuffer<uint32_t>( cmdList, mesh.indexBuffer );
            }
         }

         prevMesh = mesh.asset;
      }

      if( mesh.indexCount > 0 )
      {
         GRIS::DrawIndexed( cmdList, mesh.indexCount );
      }
      else
      {
         GRIS::Draw( cmdList, mesh.vertexCount );
      }
   }

   GRIS::EndRendering( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   RenderGraph::AddPass( cmdList );
}
}
