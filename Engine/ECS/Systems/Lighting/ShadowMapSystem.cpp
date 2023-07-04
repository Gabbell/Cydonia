#include <ECS/Systems/Lighting/ShadowMapSystem.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/Transforms.h>

#include <Graphics/StaticPipelines.h>
#include <Graphics/Scene/MaterialCache.h>

#include <Profiling.h>
#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
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

void ShadowMapSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "ShadowMapSystem" );

   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Initialize Shadow Map
   if( !scene.shadowMap )
   {
      TextureDescription texDesc;
      texDesc.width  = 2048;
      texDesc.height = 2048;
      texDesc.size   = texDesc.width * texDesc.height * sizeof( float );
      texDesc.type   = ImageType::TEXTURE_2D;
      texDesc.format = PixelFormat::D32_SFLOAT;
      texDesc.usage  = ImageUsage::SAMPLED | ImageUsage::DEPTH_STENCIL;
      texDesc.stages = PipelineStage::FRAGMENT_STAGE;
      texDesc.name   = "Shadow Map";

      scene.shadowMap = GRIS::CreateTexture( texDesc );
   }

   const CmdListHandle cmdList = GRIS::GetMainCommandList();
   CYD_GPUTRACE( cmdList, "ShadowMapSystem" );

   FramebufferInfo framebuffer;
   framebuffer.attachments.push_back(
       { PixelFormat::D32_SFLOAT, AttachmentType::DEPTH_STENCIL, LoadOp::CLEAR, StoreOp::STORE } );

   // TODO go through this loop once per light view
   GRIS::BeginRendering( cmdList, framebuffer, { scene.shadowMap } );

   GRIS::SetViewport( cmdList, {} );
   GRIS::SetScissor( cmdList, {} );

   const PipelineIndex pipIdx  = StaticPipelines::FindByName( "TERRAIN_SHADOWMAP" );
   const PipelineInfo* pipInfo = StaticPipelines::Get( pipIdx );

   GRIS::BindPipeline( cmdList, pipIdx );

   // Tracking
   std::string_view prevMesh;
   PipelineIndex prevPipeline = INVALID_PIPELINE_IDX;
   MaterialIndex prevMaterial = INVALID_MATERIAL_IDX;

   for( const auto& entityEntry : m_entities )
   {
      const RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );

      if( !renderable.isShadowCasting ) continue;

      const TransformComponent& transform = *std::get<TransformComponent*>( entityEntry.arch );
      const MaterialComponent& material   = *std::get<MaterialComponent*>( entityEntry.arch );
      const MeshComponent& mesh           = *std::get<MeshComponent*>( entityEntry.arch );

      const auto& it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "LightView 0" );
      if( it == scene.viewNames.end() )
      {
         // TODO WARNING
         CYD_ASSERT( !"Could not find main view, skipping render tick" );
         return;
      }

      const uint32_t viewIdx =
          static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );

      const glm::mat4 modelMatrix =
          Transform::GetModelMatrix( transform.scaling, transform.rotation, transform.position );

      optionalBufferBinding(
          cmdList,
          scene.viewsBuffer,
          "EnvironmentView",
          pipInfo,
          viewIdx * sizeof( SceneComponent::ViewShaderParams ),
          sizeof( SceneComponent::ViewShaderParams ) );

      // Update model transform push constant
      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::VERTEX_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

      if( prevMaterial != material.materialIdx )
      {
         m_materials.bind( cmdList, material.materialIdx, 1 /*set*/ );
         prevMaterial = material.materialIdx;
      }

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
}
}