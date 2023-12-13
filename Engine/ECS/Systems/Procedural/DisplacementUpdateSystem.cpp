#include <ECS/Systems/Procedural/DisplacementUpdateSystem.h>

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
static bool s_initialized                = false;
static PipelineIndex s_heightToNormalPip = INVALID_PIPELINE_IDX;

static void Initialize()
{
   Noise::Initialize();
   s_heightToNormalPip = StaticPipelines::FindByName( "SIMPLEX_NORMAL" );
   s_initialized       = true;
}

static void
GenerateNoise( CmdListHandle cmdList, const DisplacementComponent& noise, uint32_t layerCount )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Generate Displacement" );

   GRIS::BindPipeline( cmdList, Noise::GetPipeline( noise.type ) );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( noise.params ), &noise.params );

   GRIS::BindImage( cmdList, noise.displacementMap, 0, 0 );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   uint32_t groupsX           = static_cast<uint32_t>( std::ceil( noise.width / localSizeX ) );
   uint32_t groupsY           = static_cast<uint32_t>( std::ceil( noise.height / localSizeY ) );

   GRIS::Dispatch( cmdList, groupsX, groupsY, layerCount );
}

static void
GenerateNormals( CmdListHandle cmdList, const DisplacementComponent& noise, uint32_t layerCount )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Generate Normals" );

   GRIS::BindPipeline( cmdList, s_heightToNormalPip );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( noise.params ), &noise.params );

   GRIS::BindImage( cmdList, noise.normalMap, 0, 0 );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   uint32_t groupsX           = static_cast<uint32_t>( std::ceil( noise.width / localSizeX ) );
   uint32_t groupsY           = static_cast<uint32_t>( std::ceil( noise.height / localSizeY ) );

   GRIS::Dispatch( cmdList, groupsX, groupsY, layerCount );
}

// ================================================================================================
void DisplacementUpdateSystem::tick( double deltaS )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER_P1 );
   CYD_SCOPED_GPUTRACE( cmdList, "DisplacementUpdateSystem" );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-Write Components
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      const MaterialComponent& material     = GetComponent<MaterialComponent>( entityEntry );
      DisplacementComponent& noise          = GetComponent<DisplacementComponent>( entityEntry );

      const uint32_t layerCount = renderable.isInstanced ? renderable.maxInstanceCount : 1;

      if( noise.resolutionChanged )
      {
         SamplerInfo sampler;
         sampler.addressMode = AddressMode::CLAMP_TO_EDGE;
         sampler.magFilter   = Filter::LINEAR;
         sampler.minFilter   = Filter::LINEAR;

         GRIS::DestroyTexture( noise.displacementMap );

         TextureDescription noiseTexDesc;
         noiseTexDesc.width  = noise.width;
         noiseTexDesc.height = noise.height;
         noiseTexDesc.depth  = layerCount;
         noiseTexDesc.type   = ImageType::TEXTURE_2D_ARRAY;
         noiseTexDesc.format = PixelFormat::R32F;
         noiseTexDesc.usage  = ImageUsage::STORAGE | ImageUsage::SAMPLED;
         noiseTexDesc.stages = PipelineStage::COMPUTE_STAGE;

         noise.displacementMap = GRIS::CreateTexture( noiseTexDesc );

         m_materials.updateMaterial(
             material.materialIdx,
             MaterialCache::TextureSlot::DISPLACEMENT,
             noise.displacementMap,
             sampler );

         if( noise.generateNormals )
         {
            GRIS::DestroyTexture( noise.normalMap );

            TextureDescription normalMapDesc;
            normalMapDesc.width  = noise.width;
            normalMapDesc.height = noise.height;
            normalMapDesc.depth  = layerCount;
            normalMapDesc.type   = ImageType::TEXTURE_2D_ARRAY;
            normalMapDesc.format = PixelFormat::RGBA32F;
            normalMapDesc.usage  = ImageUsage::STORAGE | ImageUsage::SAMPLED;
            normalMapDesc.stages = PipelineStage::COMPUTE_STAGE;

            noise.normalMap = GRIS::CreateTexture( normalMapDesc );

            m_materials.updateMaterial(
                material.materialIdx,
                MaterialCache::TextureSlot::NORMAL,
                noise.normalMap,
                sampler );
         }

         noise.resolutionChanged = false;
      }

      const bool needsUpdate = noise.needsUpdate || noise.speed > 0.0f;
      if( needsUpdate )
      {
         noise.params.seed += noise.speed * static_cast<float>( deltaS );

         GenerateNoise( cmdList, noise, layerCount );

         if( noise.generateNormals )
         {
            GenerateNormals( cmdList, noise, layerCount );
         }

         noise.needsUpdate = false;
      }
   }
}
}