#include <ECS/Systems/Procedural/AtmosphereUpdateSystem.h>

#include <Graphics/Framebuffer.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/RenderGraph.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
static bool s_initialized                      = false;
static PipelineIndex s_transmittanceLUTPip     = INVALID_PIPELINE_IDX;
static PipelineIndex s_multiScatteringLUTPip   = INVALID_PIPELINE_IDX;
static PipelineIndex s_skyViewLUTPip           = INVALID_PIPELINE_IDX;
static PipelineIndex s_aerialPerspectiveLUTPip = INVALID_PIPELINE_IDX;
static PipelineIndex s_shadowVolumePip         = INVALID_PIPELINE_IDX;

static void Initialize()
{
   s_transmittanceLUTPip     = StaticPipelines::FindByName( "ATMOS_TRANSMITTANCE_LUT" );
   s_multiScatteringLUTPip   = StaticPipelines::FindByName( "ATMOS_MULTISCATTERING_LUT" );
   s_skyViewLUTPip           = StaticPipelines::FindByName( "ATMOS_SKYVIEW_LUT" );
   s_aerialPerspectiveLUTPip = StaticPipelines::FindByName( "ATMOS_AERIAL_PERSPECTIVE_LUT" );
   s_shadowVolumePip         = StaticPipelines::FindByName( "SHADOW_VOLUME_LUT" );
   s_initialized             = true;
}

static void ComputeTransmittanceLUT( CmdListHandle cmdList, AtmosphereComponent& atmos )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Compute Transmittance LUT" );

   if( !atmos.transmittanceLUT )
   {
      TextureDescription texDesc = {};
      texDesc.width              = AtmosphereComponent::TRANSMITTANCE_LUT_WIDTH;
      texDesc.height             = AtmosphereComponent::TRANSMITTANCE_LUT_HEIGHT;
      texDesc.type               = ImageType::TEXTURE_2D;
      texDesc.format             = PixelFormat::RGBA8_UNORM;
      texDesc.usage              = ImageUsage::STORAGE | ImageUsage::SAMPLED;
      texDesc.stages             = PipelineStage::COMPUTE_STAGE;
      texDesc.name               = "Transmittance LUT";

      atmos.transmittanceLUT = GRIS::CreateTexture( texDesc );
   }

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   uint32_t groupsX           = static_cast<uint32_t>(
       std::ceil( AtmosphereComponent::TRANSMITTANCE_LUT_WIDTH / localSizeX ) );
   uint32_t groupsY = static_cast<uint32_t>(
       std::ceil( AtmosphereComponent::TRANSMITTANCE_LUT_HEIGHT / localSizeY ) );

   GRIS::BindPipeline( cmdList, s_transmittanceLUTPip );

   GRIS::BindImage( cmdList, atmos.transmittanceLUT, 0 );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

   GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );
}

static void ComputeMultipleScatteringLUT( CmdListHandle cmdList, AtmosphereComponent& atmos )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Compute Multiple Scattering LUT" );

   if( !atmos.multipleScatteringLUT )
   {
      TextureDescription texDesc = {};
      texDesc.width              = AtmosphereComponent::MULTIPLE_SCATTERING_LUT_DIM;
      texDesc.height             = AtmosphereComponent::MULTIPLE_SCATTERING_LUT_DIM;
      texDesc.type               = ImageType::TEXTURE_2D;
      texDesc.format             = PixelFormat::RGBA16F;
      texDesc.usage              = ImageUsage::STORAGE | ImageUsage::SAMPLED;
      texDesc.stages             = PipelineStage::COMPUTE_STAGE;
      texDesc.name               = "Multiple Scattering LUT";

      atmos.multipleScatteringLUT = GRIS::CreateTexture( texDesc );
   }

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   uint32_t groupsX           = static_cast<uint32_t>(
       std::ceil( AtmosphereComponent::MULTIPLE_SCATTERING_LUT_DIM / localSizeX ) );
   uint32_t groupsY = static_cast<uint32_t>(
       std::ceil( AtmosphereComponent::MULTIPLE_SCATTERING_LUT_DIM / localSizeY ) );

   GRIS::BindPipeline( cmdList, s_multiScatteringLUTPip );

   GRIS::BindImage( cmdList, atmos.transmittanceLUT, 0 );
   GRIS::BindImage( cmdList, atmos.multipleScatteringLUT, 1 );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

   GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );
}

static void
ComputeSkyViewLUT( CmdListHandle cmdList, SceneComponent& scene, AtmosphereComponent& atmos )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Compute Sky-View LUT" );

   if( !atmos.skyViewLUT )
   {
      TextureDescription texDesc = {};
      texDesc.width              = AtmosphereComponent::SKYVIEW_LUT_WIDTH;
      texDesc.height             = AtmosphereComponent::SKYVIEW_LUT_HEIGHT;
      texDesc.type               = ImageType::TEXTURE_2D;
      texDesc.format             = PixelFormat::RGBA16F;
      texDesc.usage  = ImageUsage::STORAGE | ImageUsage::SAMPLED | ImageUsage::TRANSFER_SRC;
      texDesc.stages = PipelineStage::COMPUTE_STAGE;
      texDesc.name   = "Sky View LUT";

      atmos.skyViewLUT = GRIS::CreateTexture( texDesc );

      // TODO Put this environment map in its own shared component or inside the scene and make
      // it a cube map that is rendered into using reflection probes
      texDesc.usage  = ImageUsage::SAMPLED | ImageUsage::TRANSFER_DST;
      texDesc.stages = PipelineStage::FRAGMENT_STAGE;
      texDesc.name   = "Environment Map";
      scene.envMap   = GRIS::CreateTexture( texDesc );
   }

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   const uint32_t groupsX =
       static_cast<uint32_t>( std::ceil( AtmosphereComponent::SKYVIEW_LUT_WIDTH / localSizeX ) );
   const uint32_t groupsY =
       static_cast<uint32_t>( std::ceil( AtmosphereComponent::SKYVIEW_LUT_HEIGHT / localSizeY ) );

   GRIS::BindPipeline( cmdList, s_skyViewLUTPip );

   GRIS::BindUniformBuffer( cmdList, scene.inverseViewsBuffer, 0 );
   GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, 1 );

   GRIS::BindTexture( cmdList, atmos.transmittanceLUT, 2 );
   GRIS::BindTexture( cmdList, atmos.multipleScatteringLUT, 3 );
   GRIS::BindImage( cmdList, atmos.skyViewLUT, 4 );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

   GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );
}

static void ComputeAerialPerspectiveLUT(
    CmdListHandle cmdList,
    const SceneComponent& scene,
    AtmosphereComponent& atmos )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Compute Aerial Perspective LUT" );

   if( !atmos.aerialPerspectiveLUT )
   {
      TextureDescription texDesc = {};
      texDesc.width              = AtmosphereComponent::AERIAL_PERSPECTIVE_LUT_DIM;
      texDesc.height             = AtmosphereComponent::AERIAL_PERSPECTIVE_LUT_DIM;
      texDesc.depth              = AtmosphereComponent::AERIAL_PERSPECTIVE_LUT_DEPTH;
      texDesc.type               = ImageType::TEXTURE_3D;
      texDesc.format             = PixelFormat::RGBA16F;
      texDesc.usage              = ImageUsage::STORAGE | ImageUsage::SAMPLED;
      texDesc.stages             = PipelineStage::COMPUTE_STAGE;
      texDesc.name               = "Aerial Perspective LUT";

      atmos.aerialPerspectiveLUT = GRIS::CreateTexture( texDesc );
   }

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   constexpr float localSizeZ = 1.0f;
   uint32_t groupsX           = static_cast<uint32_t>(
       std::ceil( AtmosphereComponent::AERIAL_PERSPECTIVE_LUT_DIM / localSizeX ) );
   uint32_t groupsY = static_cast<uint32_t>(
       std::ceil( AtmosphereComponent::AERIAL_PERSPECTIVE_LUT_DIM / localSizeY ) );
   uint32_t groupsZ = static_cast<uint32_t>(
       std::ceil( AtmosphereComponent::AERIAL_PERSPECTIVE_LUT_DEPTH / localSizeZ ) );

   GRIS::BindPipeline( cmdList, s_aerialPerspectiveLUTPip );

   GRIS::BindUniformBuffer( cmdList, scene.viewsBuffer, 0 );
   GRIS::BindUniformBuffer( cmdList, scene.inverseViewsBuffer, 1 );
   GRIS::BindUniformBuffer( cmdList, scene.lightsBuffer, 2 );

   GRIS::BindTexture( cmdList, atmos.transmittanceLUT, 3 );
   GRIS::BindTexture( cmdList, atmos.multipleScatteringLUT, 4 );

   GRIS::BindImage( cmdList, atmos.aerialPerspectiveLUT, 5 );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

   GRIS::Dispatch( cmdList, groupsX, groupsY, groupsZ );
}

// ================================================================================================
void AtmosphereUpdateSystem::tick( double deltaS )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER_P1 );
   CYD_SCOPED_GPUTRACE( cmdList, "AtmosphereUpdateSystem" );

   // Not const because of envmap
   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      const RenderableComponent& renderable = GetComponent<RenderableComponent>( entityEntry );
      AtmosphereComponent& atmos            = GetComponent<AtmosphereComponent>( entityEntry );

      atmos.params.time += static_cast<float>( deltaS );

      if( !renderable.desc.isVisible )
      {
         continue;
      }

      if( atmos.needsUpdate )
      {
         ComputeTransmittanceLUT( cmdList, atmos );
         ComputeMultipleScatteringLUT( cmdList, atmos );
         atmos.needsUpdate = false;
      }

      ComputeSkyViewLUT( cmdList, scene, atmos );
      ComputeAerialPerspectiveLUT( cmdList, scene, atmos );

      const TextureCopyInfo texInfo = {};
      GRIS::CopyTexture( cmdList, atmos.skyViewLUT, scene.envMap, texInfo );
   }

   CYD_GPUTRACE_END( cmdList );
}
}