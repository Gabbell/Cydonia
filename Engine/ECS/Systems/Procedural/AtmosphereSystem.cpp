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
static bool s_initialized                    = false;
static PipelineIndex s_transmittanceLUTPip   = INVALID_PIPELINE_IDX;
static PipelineIndex s_multiScatteringLUTPip = INVALID_PIPELINE_IDX;
static PipelineIndex s_skyViewLUTPip         = INVALID_PIPELINE_IDX;
static PipelineIndex s_outputPip             = INVALID_PIPELINE_IDX;

static void Initialize()
{
   s_transmittanceLUTPip   = StaticPipelines::FindByName( "ATMOS_TRANSMITTANCE_LUT" );
   s_multiScatteringLUTPip = StaticPipelines::FindByName( "ATMOS_MULTISCATTERING_LUT" );
   s_skyViewLUTPip         = StaticPipelines::FindByName( "ATMOS_SKYVIEW_LUT" );
   s_outputPip             = StaticPipelines::FindByName( "ATMOS_OUTPUT" );

   s_initialized = true;
}

static void ComputeTransmittanceLUT( CmdListHandle cmdList, AtmosphereComponent& atmos )
{
   CYD_SCOPED_GPUTRACE( cmdList, "Compute Transmittance LUT" );

   if( !atmos.transmittanceLUT )
   {
      TextureDescription texDesc = {};
      texDesc.width              = AtmosphereComponent::TRANSMITTANCE_LUT_WIDTH;
      texDesc.height             = AtmosphereComponent::TRANSMITTANCE_LUT_HEIGHT;
      texDesc.size               = texDesc.width * texDesc.height * sizeof( float );
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
      texDesc.size               = texDesc.width * texDesc.height * sizeof( float );
      texDesc.type               = ImageType::TEXTURE_2D;
      texDesc.format             = PixelFormat::RGBA8_UNORM;
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
      texDesc.size               = texDesc.width * texDesc.height * sizeof( float );
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

// ================================================================================================
void AtmosphereSystem::tick( double deltaS )
{
   CYD_TRACE( "AtmosphereSystem" );

   if( !s_initialized )
   {
      Initialize();
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::POST_PROCESS );
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
   const SceneComponent::ViewShaderParams& view   = scene.views[viewIdx];
   const SceneComponent::LightShaderParams& light = scene.lights[0];

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

      atmos.params.groundRadiusMM     = 6.36f;
      atmos.params.atmosphereRadiusMM = 6.46f;

      const UploadToBufferInfo info = { 0, sizeof( AtmosphereComponent::ViewInfo ) };
      GRIS::UploadToBuffer( atmos.viewInfoBuffer, &atmos.viewInfo, info );

      if( atmos.needsUpdate )
      {
         ComputeTransmittanceLUT( cmdList, atmos );
         ComputeMultipleScatteringLUT( cmdList, atmos );
         atmos.needsUpdate = false;
      }

      ComputeSkyViewLUT( cmdList, atmos );

      CYD_GPUTRACE_BEGIN( cmdList, "Atmosphere Output" );

      constexpr float localSizeX = 16.0f;
      constexpr float localSizeY = 16.0f;
      uint32_t groupsX = static_cast<uint32_t>( std::ceil( scene.viewport.width / localSizeX ) );
      uint32_t groupsY = static_cast<uint32_t>( std::ceil( scene.viewport.height / localSizeY ) );

      // Output to color
      GRIS::BindPipeline( cmdList, s_outputPip );

      // We're using texture+samplers here instead of images to reduce banding
      GRIS::BindUniformBuffer( cmdList, atmos.viewInfoBuffer, 0 );
      GRIS::BindTexture( cmdList, atmos.transmittanceLUT, 1 );
      GRIS::BindTexture( cmdList, atmos.skyViewLUT, 2 );
      GRIS::BindImage( cmdList, scene.mainColor, 3 );
      GRIS::BindTexture( cmdList, scene.mainDepth, 4 );

      GRIS::UpdateConstantBuffer(
          cmdList, PipelineStage::COMPUTE_STAGE, 0, sizeof( atmos.params ), &atmos.params );

      GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

      CYD_GPUTRACE_END( cmdList );
   }
}
}