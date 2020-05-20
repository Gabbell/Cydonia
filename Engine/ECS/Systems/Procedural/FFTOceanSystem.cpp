#include <ECS/Systems/Procedural/FFTOceanSystem.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/Pipelines.h>

#include <ECS/Components/Procedural/FFTOceanComponent.h>

namespace CYD
{
static ComputePipelineInfo spectraGenPip;  // Phillips spectra generation pipeline
static ComputePipelineInfo heightModPip;   // Height modulation pipeline

static struct HeightModulationInfo  // Constant buffer structure for height modulation shader
{
   uint32_t width   = 0;
   uint32_t height  = 0;
   uint32_t offset  = 0;
   uint32_t stride  = 0;
   float modulation = 0;
} modInfo;

void FFTOceanSystem::tick( double /*deltaS*/ )
{
   for( const auto& compPair : m_components )
   {
      const MeshComponent& mesh = *std::get<MeshComponent*>( compPair.second );
      FFTOceanComponent& ocean  = *std::get<FFTOceanComponent*>( compPair.second );

      const CmdListHandle computeList = GRIS::CreateCommandList( COMPUTE | TRANSFER );
      GRIS::StartRecordingCommandList( computeList );

      const uint32_t resolution = ocean.spectraGenInfo.resolution;

      if( ocean.needsUpdate )
      {
         if( ocean.spectrum1 != Handle::INVALID_HANDLE ||
             ocean.spectrum2 != Handle::INVALID_HANDLE )
         {
            GRIS::DestroyTexture( ocean.spectrum1 );
            GRIS::DestroyTexture( ocean.spectrum2 );
         }

         TextureDescription spectrumDesc = {};
         spectrumDesc.size               = resolution * resolution * 4 * sizeof( float );
         spectrumDesc.width              = resolution;
         spectrumDesc.height             = resolution;
         spectrumDesc.type               = ImageType::TEXTURE_2D;
         spectrumDesc.format             = PixelFormat::RGBA32F_SFLOAT;
         spectrumDesc.usage              = ImageUsage::STORAGE;
         spectrumDesc.stages             = ShaderStage::COMPUTE_STAGE;

         ocean.spectrum1 = GRIS::CreateTexture( computeList, spectrumDesc );
         ocean.spectrum2 = GRIS::CreateTexture( computeList, spectrumDesc );

         // Generate the two Phillips spectrum textures
         GRIS::BindPipeline( computeList, spectraGenPip );
         GRIS::BindImage( computeList, ocean.spectrum1, 0, 0 );
         GRIS::BindImage( computeList, ocean.spectrum2, 0, 1 );
         GRIS::UpdateConstantBuffer(
             computeList,
             ShaderStage::COMPUTE_STAGE,
             0,
             sizeof( FFTOceanComponent::SpectraGenInfo ),
             &ocean.spectraGenInfo );
         GRIS::Dispatch( computeList, resolution, resolution, 1 );

         ocean.needsUpdate = false;
      }

      // Updating data to send as constant buffer
      modInfo.width      = resolution;
      modInfo.height     = resolution;
      modInfo.offset     = offsetof( Vertex, pos );
      modInfo.stride     = sizeof( Vertex );
      modInfo.modulation = ocean.heightModulation;

      // Modulate grid mesh with heightmap
      // GRIS::BindPipeline( computeList, pipInfo );
      // GRIS::BindBuffer( computeList, mesh.vertexBuffer, 0, 0 );
      // GRIS::BindTexture( computeList, ocean.heightField, 0, 1 );
      // GRIS::UpdateConstantBuffer(
      //     computeList, COMPUTE_STAGE, 0, sizeof( HeightModulationInfo ), &modInfo );
      // GRIS::Dispatch( computeList, ocean.resolution, ocean.resolution, 1 );

      GRIS::EndRecordingCommandList( computeList );
      
      GRIS::SubmitCommandList( computeList );
      GRIS::WaitOnCommandList( computeList );

      GRIS::DestroyCommandList( computeList );
   }
}

bool FFTOceanSystem::init()
{
   // Heightmap modulation descriptor set
   DescriptorSetLayoutInfo heightModSet = {};  // Set 0

   const ShaderResourceInfo vertexDataInfo = {
       ShaderResourceType::STORAGE, ShaderStage::COMPUTE_STAGE, 0 };
   const ShaderResourceInfo heightMapInfo = {
       ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 1 };

   heightModSet.shaderResources.push_back( vertexDataInfo );
   heightModSet.shaderResources.push_back( heightMapInfo );

   const PushConstantRange modInfoRange = {
       ShaderStage::COMPUTE_STAGE, 0, sizeof( HeightModulationInfo ) };

   // Heightmap modulation pipeline initialization
   heightModPip.shader = "HEIGHT_MODULATION_COMP";
   heightModPip.pipLayout.descSets.push_back( heightModSet );
   heightModPip.pipLayout.ranges.push_back( modInfoRange );

   // Phillips spectra generation descriptor set
   DescriptorSetLayoutInfo spectraGenSet = {};  // Set 0

   const ShaderResourceInfo spectrum1Info = {
       ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 0 };
   const ShaderResourceInfo spectrum2Info = {
       ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 1 };

   spectraGenSet.shaderResources.push_back( spectrum1Info );
   spectraGenSet.shaderResources.push_back( spectrum2Info );

   const PushConstantRange spectraGenParamsRange = {
       ShaderStage::COMPUTE_STAGE, 0, sizeof( FFTOceanComponent::SpectraGenInfo ) };

   // Phillips spectra generation pipeline initialization
   spectraGenPip.shader = "FFTOCEAN_SPECTRA_COMP";
   spectraGenPip.pipLayout.descSets.push_back( spectraGenSet );
   spectraGenPip.pipLayout.ranges.push_back( spectraGenParamsRange );

   return true;
}

void FFTOceanSystem::uninit() {}
}
