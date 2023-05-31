#include <ECS/Systems/Procedural/NoiseGenerationSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include <Graphics/Scene/MaterialCache.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/InputComponent.h>

namespace CYD
{
// ================================================================================================
void NoiseGenerationSystem::tick( double deltaS )
{
   // Start command list recording
   const CmdListHandle cmdList = GRIS::CreateCommandList( COMPUTE, "NoiseGenerationSystem" );

   GRIS::StartRecordingCommandList( cmdList );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-Write Components
      NoiseComponent& noise       = *std::get<NoiseComponent*>( entityEntry.arch );
      MaterialComponent& material = *std::get<MaterialComponent*>( entityEntry.arch );

      // Read-only Components
      const InputComponent& input = m_ecs->getSharedComponent<InputComponent>();

      if( input.resolutionChanged )
      {
         GRIS::DestroyTexture( noise.texture );

         TextureDescription noiseTexDesc;
         noiseTexDesc.width  = input.windowWidth;
         noiseTexDesc.height = input.windowHeight;
         noiseTexDesc.size   = noiseTexDesc.width * noiseTexDesc.height * sizeof( float );
         noiseTexDesc.type   = ImageType::TEXTURE_2D;
         noiseTexDesc.format = PixelFormat::RGBA8_UNORM;
         noiseTexDesc.usage  = ImageUsage::STORAGE | ImageUsage::SAMPLED;
         noiseTexDesc.stages = PipelineStage::COMPUTE_STAGE;

         noise.texture = GRIS::CreateTexture( noiseTexDesc );

         noise.params.width  = static_cast<float>( input.windowWidth );
         noise.params.height = static_cast<float>( input.windowHeight );

         m_materials.updateMaterial( material.materialIdx, noise.texture, 0 );
      }

      if( noise.needsUpdate )
      {
         noise.params.seed += static_cast<float>( deltaS ) + 1.0f;
         noise.params.frequency = 8.0f;
         noise.params.scale     = 2.0f;
         noise.params.exponent  = 0.3f;
         noise.params.normalize = true;
         noise.params.absolute  = false;
         noise.params.octaves   = 3;

         GRIS::BindPipeline( cmdList, material.pipelineIdx );

         GRIS::UpdateConstantBuffer(
             cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( noise.params ), &noise.params );
         GRIS::BindImage( cmdList, noise.texture, 0, 0 );

         constexpr float localSizeX = 16.0f;
         constexpr float localSizeY = 16.0f;
         uint32_t groupsX = static_cast<uint32_t>( std::ceil( noise.params.width / localSizeX ) );
         uint32_t groupsY = static_cast<uint32_t>( std::ceil( noise.params.height / localSizeY ) );

         GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

         // If this noise is constant/static, it does not need to update anymore
         if( noise.constant ) noise.needsUpdate = false;
      }
   }

   GRIS::EndRecordingCommandList( cmdList );

   RenderGraph::AddPass( cmdList );
}
}