#include <ECS/Systems/Lighting/ShadowMapSystem.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/Framebuffer.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/Utility/Transforms.h>
#include <Graphics/Utility/ShadowMapping.h>

#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
static bool s_initialized                = false;
static PipelineIndex s_shadowmapPipeline = INVALID_PIPELINE_IDX;

static void Initialize()
{
   s_shadowmapPipeline = StaticPipelines::FindByName( "TERRAIN_SHADOWMAP" );  // TODO
   s_initialized       = true;
}

void ShadowMapSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER_P2 );
   CYD_SCOPED_GPUTRACE( cmdList, "ShadowMapSystem" );

   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();
   for( uint32_t i = 0; i < SceneComponent::MAX_SHADOW_MAPS; ++i )
   {
      const Framebuffer& shadowMapFB = scene.shadowMapFBs[i];
      if( shadowMapFB.isValid() )
      {
         // Draw into every cascade
         for( uint32_t cascadeIndex = 0; cascadeIndex < ShadowMapping::MAX_CASCADES;
              ++cascadeIndex )
         {
#if CYD_GPU_PROFILING
            const std::string cascadeString =
                "Light " + std::to_string( i ) + ", Cascade " + std::to_string( cascadeIndex );
            CYD_SCOPED_GPUTRACE( cmdList, cascadeString.c_str() );
#endif

            GRIS::BeginRendering( cmdList, shadowMapFB, cascadeIndex );

            GRIS::SetViewport( cmdList, {} );
            GRIS::SetScissor( cmdList, {} );

            const PipelineInfo& pipInfo = *StaticPipelines::Get( s_shadowmapPipeline );
            GRIS::BindPipeline( cmdList, s_shadowmapPipeline );

            // Tracking
            MeshIndex prevMesh         = INVALID_MESH_IDX;
            MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;

            for( const auto& entityEntry : m_entities )
            {
               const RenderableComponent& renderable =
                   GetComponent<RenderableComponent>( entityEntry );
               const TransformComponent& transform =
                   GetComponent<TransformComponent>( entityEntry );
               const MaterialComponent& material = GetComponent<MaterialComponent>( entityEntry );
               const MeshComponent& mesh         = GetComponent<MeshComponent>( entityEntry );

               if( !renderable.desc.isVisible || !renderable.desc.isShadowCasting )
               {
                  continue;
               }

               if( renderable.pipelineIdx == INVALID_PIPELINE_IDX ||
                   mesh.meshIdx == INVALID_MESH_IDX ||
                   material.materialIdx == INVALID_MATERIAL_IDX )
               {
                  continue;
               }

               // Vertex and index buffers
               // ==========================================================================================
               if( prevMesh != mesh.meshIdx )
               {
                  m_meshes.bind( cmdList, mesh.meshIdx );

                  prevMesh = mesh.meshIdx;
               }

               // Material
               // ==========================================================================================
               if( prevMaterial != material.materialIdx )
               {
                  m_materials.bindSlot(
                      cmdList,
                      material.materialIdx,
                      MaterialCache::TextureSlot::DISPLACEMENT,
                      5,
                      1 /*set*/ );

                  prevMaterial = material.materialIdx;
               }

               // Optional Buffers
               // ==========================================================================================
               const glm::mat4 modelMatrix = Transform::GetModelMatrix(
                   transform.scaling, transform.rotation, transform.position );

               GRIS::UpdateConstantBuffer(
                   cmdList,
                   PipelineStage::VERTEX_STAGE,
                   0,
                   sizeof( modelMatrix ),
                   &modelMatrix );

               GRIS::UpdateConstantBuffer(
                   cmdList,
                   PipelineStage::VERTEX_STAGE,
                   64,
                   sizeof( cascadeIndex ),
                   &cascadeIndex );

               GRIS::NamedBufferBinding( cmdList, scene.viewsBuffer, "VIEWS", pipInfo );
               GRIS::NamedBufferBinding( cmdList, scene.shadowMapsBuffer, "SHADOWMAPS", pipInfo );
               GRIS::NamedBufferBinding( cmdList, scene.frustumsBuffer, "FRUSTUMS", pipInfo );

               if( renderable.isInstanced )
               {
                  CYD_ASSERT( renderable.instancesBuffer && "Invalid instance buffer" );
                  GRIS::NamedBufferBinding(
                      cmdList, renderable.instancesBuffer, "INSTANCES", pipInfo );
               }

               if( renderable.isTessellated )
               {
                  CYD_ASSERT(
                      renderable.tessellationBuffer && "Invalid tessellation params buffer" );
                  GRIS::NamedBufferBinding(
                      cmdList, renderable.tessellationBuffer, "TESSELLATION", pipInfo );
               }

               if( renderable.instanceCount )
               {
                  if( mesh.indexCount )
                  {
                     GRIS::DrawIndexedInstanced(
                         cmdList, mesh.indexCount, renderable.instanceCount );
                  }
                  else
                  {
                     GRIS::DrawInstanced( cmdList, mesh.vertexCount, renderable.instanceCount );
                  }
               }
               else
               {
                  if( mesh.indexCount )
                  {
                     GRIS::DrawIndexed( cmdList, mesh.indexCount );
                  }
                  else
                  {
                     GRIS::Draw( cmdList, mesh.vertexCount );
                  }
               }
            }

            GRIS::EndRendering( cmdList );
         }
      }
   }
}
}