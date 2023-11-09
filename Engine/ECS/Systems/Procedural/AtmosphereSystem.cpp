#include <ECS/Systems/Procedural/AtmosphereSystem.h>

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

static void Initialize()
{
   s_transmittanceLUTPip     = StaticPipelines::FindByName( "ATMOS_TRANSMITTANCE_LUT" );
   s_multiScatteringLUTPip   = StaticPipelines::FindByName( "ATMOS_MULTISCATTERING_LUT" );
   s_skyViewLUTPip           = StaticPipelines::FindByName( "ATMOS_SKYVIEW_LUT" );
   s_aerialPerspectiveLUTPip = StaticPipelines::FindByName( "ATMOS_AERIAL_PERSPECTIVE_LUT" );
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

static void ComputeSkyViewLUT( CmdListHandle cmdList, AtmosphereComponent& atmos )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Compute Sky-View LUT" );

   if( !atmos.skyViewLUT )
   {
      TextureDescription texDesc = {};
      texDesc.width              = AtmosphereComponent::SKYVIEW_LUT_WIDTH;
      texDesc.height             = AtmosphereComponent::SKYVIEW_LUT_HEIGHT;
      texDesc.type               = ImageType::TEXTURE_2D;
      texDesc.format             = PixelFormat::RGBA16F;
      texDesc.usage              = ImageUsage::STORAGE | ImageUsage::SAMPLED;
      texDesc.stages             = PipelineStage::COMPUTE_STAGE;
      texDesc.name               = "Sky View LUT";

      atmos.skyViewLUT = GRIS::CreateTexture( texDesc );
   }

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   uint32_t groupsX =
       static_cast<uint32_t>( std::ceil( AtmosphereComponent::SKYVIEW_LUT_WIDTH / localSizeX ) );
   uint32_t groupsY =
       static_cast<uint32_t>( std::ceil( AtmosphereComponent::SKYVIEW_LUT_HEIGHT / localSizeY ) );

   GRIS::BindPipeline( cmdList, s_skyViewLUTPip );

   GRIS::BindUniformBuffer( cmdList, atmos.viewInfoBuffer, 0 );
   GRIS::BindTexture( cmdList, atmos.transmittanceLUT, 1 );
   GRIS::BindTexture( cmdList, atmos.multipleScatteringLUT, 2 );
   GRIS::BindImage( cmdList, atmos.skyViewLUT, 3 );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

   GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );
}

static void ComputeAerialPerspectiveLUT( CmdListHandle cmdList, AtmosphereComponent& atmos )
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

   GRIS::BindUniformBuffer( cmdList, atmos.viewInfoBuffer, 0 );
   GRIS::BindTexture( cmdList, atmos.transmittanceLUT, 1 );
   GRIS::BindTexture( cmdList, atmos.multipleScatteringLUT, 2 );
   GRIS::BindImage( cmdList, atmos.aerialPerspectiveLUT, 3 );

   GRIS::UpdateConstantBuffer(
       cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

   GRIS::Dispatch( cmdList, groupsX, groupsY, groupsZ );
}

// ================================================================================================
void AtmosphereSystem::tick( double deltaS )
{
   CYD_TRACE();

   if( !s_initialized )
   {
      Initialize();
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER );
   CYD_SCOPED_GPUTRACE( cmdList, "AtmosphereSystem" );

   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   const auto& it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "MAIN" );
   if( it == scene.viewNames.end() )
   {
      // TODO WARNING
      CYD_ASSERT( !"Could not find main view, skipping render tick" );
      return;
   }
   const uint32_t viewIdx = static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );
   const SceneComponent::ViewShaderParams& view               = scene.views[viewIdx];
   const SceneComponent::InverseViewShaderParams& inverseView = scene.inverseViews[viewIdx];
   const SceneComponent::LightShaderParams& light             = scene.lights[0];

   // Iterate through entities
   for( const auto& entityEntry : m_entities )
   {
      // Read-only components
      AtmosphereComponent& atmos = *std::get<AtmosphereComponent*>( entityEntry.arch );

      if( !atmos.viewInfoBuffer )
      {
         atmos.viewInfoBuffer = GRIS::CreateUniformBuffer(
             sizeof( AtmosphereComponent::ViewInfo ), "Atmosphere View Info" );
      }

      atmos.viewInfo.invProj  = glm::inverse( view.projMat );
      atmos.viewInfo.invView  = glm::inverse( view.viewMat );
      atmos.viewInfo.viewPos  = view.position;
      atmos.viewInfo.lightDir = light.direction;

      atmos.params.nearClip = 10000.0f;  // Get from camera, remember it's also reversed-Z
      atmos.params.farClip  = 0.1f;

      atmos.params.time += static_cast<float>( deltaS );

      const UploadToBufferInfo info = { 0, sizeof( AtmosphereComponent::ViewInfo ) };
      GRIS::UploadToBuffer( atmos.viewInfoBuffer, &atmos.viewInfo, info );

      if( atmos.needsUpdate )
      {
         ComputeTransmittanceLUT( cmdList, atmos );
         ComputeMultipleScatteringLUT( cmdList, atmos );
         atmos.needsUpdate = false;
      }

      ComputeSkyViewLUT( cmdList, atmos );
      ComputeAerialPerspectiveLUT( cmdList, atmos );
   }

   CYD_GPUTRACE_END( cmdList );
}
}