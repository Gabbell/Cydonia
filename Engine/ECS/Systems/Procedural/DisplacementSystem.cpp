#include <ECS/Systems/Procedural/DisplacementSystem.h>

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
   s_heightToNormalPip = StaticPipelines::FindByName( "HEIGHT_TO_NORMAL" );
   s_initialized       = true;
}

// TODO Make GenerateNoise and GenerateNormals run in parallel using the same noise function
// Better normals, maybe equal performance?
static void
GenerateNoise( CmdListHandle cmdList, const DisplacementComponent& noise, uint32_t layerCount )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Generate Noise" );

   GRIS::BindPipeline( cmdList, Noise::GetPipeline( noise.type ) );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( noise.params ), &noise.params );

   GRIS::BindImage( cmdList, noise.noiseTex, 0, 0 );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   uint32_t groupsX           = static_cast<uint32_t>( std::ceil( noise.width / localSizeX ) );
   uint32_t groupsY           = static_cast<uint32_t>( std::ceil( noise.height / localSizeY ) );

   GRIS::Dispatch( cmdList, groupsX, groupsY, layerCount );
}

static void GenerateNormals(
    CmdListHandle cmdList,
    const TransformComponent& transform,
    const DisplacementComponent& noise,
    uint32_t layerCount )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Generate Normals" );

   GRIS::BindPipeline( cmdList, s_heightToNormalPip );

   GRIS::BindTexture( cmdList, noise.noiseTex, 0, 0 );
   GRIS::BindImage( cmdList, noise.normalMap, 1, 0 );

   // The full displacement scaling factor is required to properly calculate the normals
   const float scale = noise.params.scale * transform.scaling.y;
   GRIS::UpdateConstantBuffer( cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( float ), &scale );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   uint32_t groupsX           = static_cast<uint32_t>( std::ceil( noise.width / localSizeX ) );
   uint32_t groupsY           = static_cast<uint32_t>( std::ceil( noise.height / localSizeY ) );

   GRIS::Dispatch( cmdList, groupsX, groupsY, layerCount );
}

// ================================================================================================
void DisplacementSystem::tick( double deltaS )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   // Start command list recording
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER );
   CYD_SCOPED_GPUTRACE( cmdList, "DisplacementSystem" );

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-Write Components
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      const TransformComponent& transform   = GetComponent<TransformComponent>( entityEntry );
      const MaterialComponent& material     = GetComponent<MaterialComponent>( entityEntry );
      DisplacementComponent& noise          = GetComponent<DisplacementComponent>( entityEntry );

      const uint32_t layerCount = renderable.isInstanced ? renderable.instanceCount : 1;

      if( noise.resolutionChanged )
      {
         GRIS::DestroyTexture( noise.noiseTex );

         TextureDescription noiseTexDesc;
         noiseTexDesc.width  = noise.width;
         noiseTexDesc.height = noise.height;
         noiseTexDesc.depth  = layerCount;
         noiseTexDesc.type   = ImageType::TEXTURE_2D_ARRAY;
         noiseTexDesc.format = PixelFormat::R32F;
         noiseTexDesc.usage  = ImageUsage::STORAGE | ImageUsage::SAMPLED;
         noiseTexDesc.stages = PipelineStage::COMPUTE_STAGE;

         noise.noiseTex = GRIS::CreateTexture( noiseTexDesc );

         m_materials.updateMaterial(
             material.materialIdx, MaterialCache::TextureSlot::DISPLACEMENT, noise.noiseTex );

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
                material.materialIdx, MaterialCache::TextureSlot::NORMAL, noise.normalMap );
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
            GenerateNormals( cmdList, transform, noise, layerCount );
         }

         noise.needsUpdate = false;
      }
   }
}
}