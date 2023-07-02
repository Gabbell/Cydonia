#include <ECS/Systems/Procedural/ProceduralDisplacementSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include <Graphics/StaticPipelines.h>

#include <Graphics/Utility/Noise.h>

#include <Graphics/Scene/MaterialCache.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/InputComponent.h>

#include <Profiling.h>

namespace CYD
{
// ================================================================================================
ProceduralDisplacementSystem::ProceduralDisplacementSystem( MaterialCache& materials )
    : m_materials( materials )
{
   Noise::Initialize();
}

ProceduralDisplacementSystem::~ProceduralDisplacementSystem() { Noise::Uninitialize(); }

// ================================================================================================
void ProceduralDisplacementSystem::tick( double deltaS )
{
   CYD_TRACE( "ProceduralDisplacementSystem" );

   // Start command list recording
   const CmdListHandle cmdList = GRIS::CreateCommandList( COMPUTE, "ProceduralDisplacementSystem" );
   CYD_GPUTRACE( cmdList, "ProceduralDisplacementSystem" );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-Write Components
      ProceduralDisplacementComponent& noise =
          *std::get<ProceduralDisplacementComponent*>( entityEntry.arch );

      const MaterialComponent& material =
          *std::get<MaterialComponent*>( entityEntry.arch );

      if( noise.resolutionChanged )
      {
         GRIS::DestroyTexture( noise.texture );

         TextureDescription noiseTexDesc;
         noiseTexDesc.width  = noise.width;
         noiseTexDesc.height = noise.height;
         noiseTexDesc.size   = noiseTexDesc.width * noiseTexDesc.height * sizeof( float );
         noiseTexDesc.type   = ImageType::TEXTURE_2D;
         noiseTexDesc.format = PixelFormat::R32F;
         noiseTexDesc.usage  = ImageUsage::STORAGE | ImageUsage::SAMPLED;
         noiseTexDesc.stages = PipelineStage::COMPUTE_STAGE;

         noise.texture = GRIS::CreateTexture( noiseTexDesc );

         m_materials.updateMaterial( material.materialIdx, noise.texture, Material::DISPLACEMENT );

         noise.resolutionChanged = false;
      }

      const bool needUpdate = noise.needsUpdate || noise.timeMultiplier > 0.0f;
      if( needUpdate )
      {
         noise.params.seed += noise.timeMultiplier * static_cast<float>( deltaS );

         GRIS::BindPipeline( cmdList, Noise::GetPipeline( noise.type ) );

         GRIS::UpdateConstantBuffer(
             cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( noise.params ), &noise.params );

         GRIS::BindImage( cmdList, noise.texture, 0, 0 );

         constexpr float localSizeX = 16.0f;
         constexpr float localSizeY = 16.0f;
         uint32_t groupsX = static_cast<uint32_t>( std::ceil( noise.width / localSizeX ) );
         uint32_t groupsY = static_cast<uint32_t>( std::ceil( noise.height / localSizeY ) );

         GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

         noise.needsUpdate = false;
      }
   }

   RenderGraph::AddPass( cmdList );
}
}