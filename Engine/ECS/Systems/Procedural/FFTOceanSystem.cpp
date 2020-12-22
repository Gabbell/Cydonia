#include <ECS/Systems/Procedural/FFTOceanSystem.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/PipelineInfos.h>

#include <ECS/Components/Procedural/FFTOceanComponent.h>

#include <Algorithms/BitManipulation.h>

#include <cmath>
#include <numeric>

namespace CYD
{
static ComputePipelineInfo spectraGenPip;            // Phillips spectra generation pipeline
static ComputePipelineInfo fourierComponentsPip;     // X-Y-Z Fourier components generation pipeline
static ComputePipelineInfo butterflyTexPip;          // Butterfly texture generation pipeline
static ComputePipelineInfo butterflyPip;             // Butterfly operations pipeline
static ComputePipelineInfo inversionPermutationPip;  // Inversion and permutation pipeline

enum class FourierComponent  // To compute the displacement for a specific component
{
   X,
   Y,
   Z
};

static void computeDisplacement(
    CmdListHandle cmdList,
    const RenderableComponent& renderable,
    FFTOceanComponent& ocean,
    FourierComponent fourierComponent )
{
   const uint32_t resolution     = ocean.parameters.resolution;
   const uint32_t numberOfStages = static_cast<uint32_t>( std::log2( resolution ) );

   // Cooley-Tukey Radix-2 FFT GPU algorithm
   GRIS::BindPipeline( cmdList, butterflyPip );

   GRIS::BindImage( cmdList, ocean.butterflyTexture, 0, 0 );

   switch( fourierComponent )
   {
      case FourierComponent::X:
         GRIS::BindImage( cmdList, ocean.fourierComponentsX, 0, 1 );
         break;
      case FourierComponent::Y:
         GRIS::BindImage( cmdList, ocean.fourierComponentsY, 0, 1 );
         break;
      case FourierComponent::Z:
         GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 0, 1 );
         break;
   }

   GRIS::BindImage( cmdList, ocean.pingpongTex, 0, 2 );

   ocean.parameters.pingpong = 0;

   ocean.parameters.direction = 0;
   for( uint32_t i = 0; i < numberOfStages; ++i )
   {
      ocean.parameters.stage = i;

      GRIS::UpdateConstantBuffer(
          cmdList,
          ShaderStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::Parameters ),
          &ocean.parameters );

      GRIS::Dispatch( cmdList, resolution / 16, resolution / 16, 1 );

      // Horizontal butterfly shaderpass
      ocean.parameters.pingpong = ( ++ocean.parameters.pingpong ) % 2;
   }

   ocean.parameters.direction = 1;
   for( uint32_t i = 0; i < numberOfStages; ++i )
   {
      ocean.parameters.stage = i;

      GRIS::UpdateConstantBuffer(
          cmdList,
          ShaderStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::Parameters ),
          &ocean.parameters );

      GRIS::Dispatch( cmdList, resolution / 16, resolution / 16, 1 );

      // Vertical butterfly shaderpass
      ocean.parameters.pingpong = ( ++ocean.parameters.pingpong ) % 2;
   }

   // Inversion and permutation shaderpass
   GRIS::BindPipeline( cmdList, inversionPermutationPip );

   GRIS::BindImage( cmdList, renderable.displacement, 0, 0 );

   switch( fourierComponent )
   {
      case FourierComponent::X:
         ocean.parameters.componentMask = 0x8;
         GRIS::BindImage( cmdList, ocean.fourierComponentsX, 0, 1 );
         break;
      case FourierComponent::Y:
         ocean.parameters.componentMask = 0x4;
         GRIS::BindImage( cmdList, ocean.fourierComponentsY, 0, 1 );
         break;
      case FourierComponent::Z:
         ocean.parameters.componentMask = 0x2;
         GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 0, 1 );
         break;
   }

   GRIS::BindImage( cmdList, ocean.pingpongTex, 0, 2 );

   GRIS::UpdateConstantBuffer(
       cmdList,
       ShaderStage::COMPUTE_STAGE,
       0,
       sizeof( FFTOceanComponent::Parameters ),
       &ocean.parameters );

   GRIS::Dispatch( cmdList, resolution / 16, resolution / 16, 1 );
}

void FFTOceanSystem::tick( double deltaS )
{
   for( const auto& entityEntry : m_entities )
   {
      RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );
      FFTOceanComponent& ocean        = *std::get<FFTOceanComponent*>( entityEntry.arch );

      // Updating time elapsed
      ocean.parameters.time += static_cast<float>( deltaS );

      const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS | COMPUTE | TRANSFER );

      GRIS::StartRecordingCommandList( cmdList );

      const uint32_t resolution     = ocean.parameters.resolution;
      const uint32_t numberOfStages = static_cast<uint32_t>( std::log2( resolution ) );

      // Recreating textures if the resolution changed
      // ===========================================================================================
      if( ocean.resolutionChanged )
      {
         // If resolution changed, we need to recreate the textures
         GRIS::DestroyTexture( ocean.spectrum1 );
         GRIS::DestroyTexture( ocean.spectrum2 );
         GRIS::DestroyTexture( ocean.butterflyTexture );
         GRIS::DestroyBuffer( ocean.bitReversedIndices );

         GRIS::DestroyTexture( ocean.fourierComponentsX );
         GRIS::DestroyTexture( ocean.fourierComponentsY );
         GRIS::DestroyTexture( ocean.fourierComponentsZ );
         GRIS::DestroyTexture( ocean.pingpongTex );

         // TODO In our case, if the entity would have another component modify this displacement
         // texture, there would be no safeguard
         GRIS::DestroyTexture( renderable.displacement );

         // TODO Size based on dimensions and pixel format
         TextureDescription rgTexDesc = {};
         rgTexDesc.size               = resolution * resolution * 2 * sizeof( float );
         rgTexDesc.width              = resolution;
         rgTexDesc.height             = resolution;
         rgTexDesc.type               = ImageType::TEXTURE_2D;
         rgTexDesc.format             = PixelFormat::RG32F;
         rgTexDesc.usage              = ImageUsage::STORAGE;
         rgTexDesc.stages             = ShaderStage::COMPUTE_STAGE;

         ocean.spectrum1 = GRIS::CreateTexture( cmdList, rgTexDesc );
         ocean.spectrum2 = GRIS::CreateTexture( cmdList, rgTexDesc );

         ocean.fourierComponentsY = GRIS::CreateTexture( cmdList, rgTexDesc );
         ocean.fourierComponentsX = GRIS::CreateTexture( cmdList, rgTexDesc );
         ocean.fourierComponentsZ = GRIS::CreateTexture( cmdList, rgTexDesc );

         ocean.pingpongTex = GRIS::CreateTexture( cmdList, rgTexDesc );

         TextureDescription dispTexDesc = {};
         dispTexDesc.size               = resolution * resolution * 4 * sizeof( float );
         dispTexDesc.width              = resolution;
         dispTexDesc.height             = resolution;
         dispTexDesc.type               = ImageType::TEXTURE_2D;
         dispTexDesc.format             = PixelFormat::RGBA32F;
         dispTexDesc.usage              = ImageUsage::STORAGE;
         dispTexDesc.stages             = ShaderStage::COMPUTE_STAGE | ShaderStage::VERTEX_STAGE;

         renderable.displacement = GRIS::CreateTexture( cmdList, dispTexDesc );

         TextureDescription butterflyDesc = {};
         butterflyDesc.size               = resolution * numberOfStages * 4 * sizeof( float );
         butterflyDesc.width              = numberOfStages;
         butterflyDesc.height             = resolution;
         butterflyDesc.type               = ImageType::TEXTURE_2D;
         butterflyDesc.format             = PixelFormat::RGBA32F;
         butterflyDesc.usage              = ImageUsage::STORAGE;
         butterflyDesc.stages             = ShaderStage::COMPUTE_STAGE;

         ocean.butterflyTexture = GRIS::CreateTexture( cmdList, butterflyDesc );

         ocean.bitReversedIndices = GRIS::CreateBuffer( sizeof( uint32_t ) * resolution );

         ocean.resolutionChanged = false;

         // Since the resolution changed, we force an update of the pre-computed textures
         ocean.needsUpdate = true;
      }

      // Generating pre-computed textures (independent)
      // ===========================================================================================
      if( ocean.needsUpdate )
      {
         // Generate the two Phillips spectrum textures
         GRIS::BindPipeline( cmdList, spectraGenPip );

         GRIS::UpdateConstantBuffer(
             cmdList,
             ShaderStage::COMPUTE_STAGE,
             0,
             sizeof( FFTOceanComponent::Parameters ),
             &ocean.parameters );

         GRIS::BindImage( cmdList, ocean.spectrum1, 0, 0 );
         GRIS::BindImage( cmdList, ocean.spectrum2, 0, 1 );
         GRIS::Dispatch( cmdList, resolution / 16, resolution / 16, 1 );

         // Generating Butterfly texture
         std::vector<uint32_t> indices( resolution );  // Bit-reversed indices
         std::iota( indices.begin(), indices.end(), static_cast<uint32_t>( 0 ) );
         EMP::BitReversalPermutation( indices );

         const size_t indicesDataSize = indices.size() * sizeof( indices[0] );
         GRIS::CopyToBuffer( ocean.bitReversedIndices, indices.data(), 0, indicesDataSize );

         GRIS::BindPipeline( cmdList, butterflyTexPip );

         GRIS::UpdateConstantBuffer(
             cmdList,
             ShaderStage::COMPUTE_STAGE,
             0,
             sizeof( FFTOceanComponent::Parameters ),
             &ocean.parameters );

         GRIS::BindImage( cmdList, ocean.butterflyTexture, 0, 0 );
         GRIS::BindBuffer( cmdList, ocean.bitReversedIndices, 0, 1 );
         GRIS::Dispatch( cmdList, numberOfStages, resolution / 16, 1 );

         ocean.needsUpdate = false;
      }

      // Generating time-dependent textures
      // ===========================================================================================

      // Generating Fourier components time-dependent textures (dy, dx, dz)
      GRIS::BindPipeline( cmdList, fourierComponentsPip );

      GRIS::UpdateConstantBuffer(
          cmdList,
          ShaderStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::Parameters ),
          &ocean.parameters );

      GRIS::BindImage( cmdList, ocean.fourierComponentsY, 0, 0 );
      GRIS::BindImage( cmdList, ocean.fourierComponentsX, 0, 1 );
      GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 0, 2 );
      GRIS::BindImage( cmdList, ocean.spectrum1, 0, 3 );
      GRIS::BindImage( cmdList, ocean.spectrum2, 0, 4 );
      GRIS::Dispatch( cmdList, resolution / 16, resolution / 16, 1 );

      computeDisplacement( cmdList, renderable, ocean, FourierComponent::X );
      computeDisplacement( cmdList, renderable, ocean, FourierComponent::Y );
      computeDisplacement( cmdList, renderable, ocean, FourierComponent::Z );

      GRIS::EndRecordingCommandList( cmdList );

      GRIS::SubmitCommandList( cmdList );
      GRIS::WaitOnCommandList( cmdList ); // TODO BAD

      GRIS::DestroyCommandList( cmdList );
   }
}

bool FFTOceanSystem::FFTOceanSystem()
{
   // FFT Ocean parameters push constant range. This push constant is used in every FFTOCEAN shaders
   const PushConstantRange oceanParamsRange = {
       ShaderStage::COMPUTE_STAGE, 0, sizeof( FFTOceanComponent::Parameters )};

   // Phillips spectra generation pipeline
   // ==============================================================================================
   {
      DescriptorSetLayoutInfo spectraGenSet = {};  // Set 0

      const ShaderResourceInfo spectrum1Info = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 0};
      const ShaderResourceInfo spectrum2Info = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 1};

      spectraGenSet.shaderResources.push_back( spectrum1Info );
      spectraGenSet.shaderResources.push_back( spectrum2Info );

      // Phillips spectra generation pipeline initialization
      spectraGenPip.shader = "FFTOCEAN_SPECTRA_COMP";
      spectraGenPip.pipLayout.descSets.push_back( spectraGenSet );
      spectraGenPip.pipLayout.ranges.push_back( oceanParamsRange );
   }

   // X-Y-Z Fourier components generation pipeline
   // ==============================================================================================
   {
      DescriptorSetLayoutInfo fourierComponentsSet = {};  // Set 0

      const ShaderResourceInfo fourierYInfo = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 0};
      const ShaderResourceInfo fourierXInfo = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 1};
      const ShaderResourceInfo fourierZInfo = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 2};

      const ShaderResourceInfo spectrum1Info = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 3};
      const ShaderResourceInfo spectrum2Info = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 4};

      fourierComponentsSet.shaderResources.push_back( fourierYInfo );
      fourierComponentsSet.shaderResources.push_back( fourierXInfo );
      fourierComponentsSet.shaderResources.push_back( fourierZInfo );

      fourierComponentsSet.shaderResources.push_back( spectrum1Info );
      fourierComponentsSet.shaderResources.push_back( spectrum2Info );

      fourierComponentsPip.shader = "FFTOCEAN_FOURIERCOMPONENTS_COMP";
      fourierComponentsPip.pipLayout.descSets.push_back( fourierComponentsSet );
      fourierComponentsPip.pipLayout.ranges.push_back( oceanParamsRange );
   }

   // Butterfly texture generation pipeline
   // ==============================================================================================
   {
      DescriptorSetLayoutInfo butterflyTexSet = {};  // Set 0

      const ShaderResourceInfo butterflyTexInfo = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 0};
      const ShaderResourceInfo bitReversedIndicesInfo = {
          ShaderResourceType::STORAGE, ShaderStage::COMPUTE_STAGE, 1};

      butterflyTexSet.shaderResources.push_back( butterflyTexInfo );
      butterflyTexSet.shaderResources.push_back( bitReversedIndicesInfo );

      butterflyTexPip.shader = "FFTOCEAN_BUTTERFLYTEX_COMP";
      butterflyTexPip.pipLayout.descSets.push_back( butterflyTexSet );
      butterflyTexPip.pipLayout.ranges.push_back( oceanParamsRange );
   }

   // Butterfly operations pipeline
   // ==============================================================================================
   {
      DescriptorSetLayoutInfo butterflySet = {};  // Set 0

      const ShaderResourceInfo butterflyTexInfo = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 0};
      const ShaderResourceInfo pingpong0Info = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 1};
      const ShaderResourceInfo pingpong1Info = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 2};

      butterflySet.shaderResources.push_back( butterflyTexInfo );
      butterflySet.shaderResources.push_back( pingpong0Info );
      butterflySet.shaderResources.push_back( pingpong1Info );

      butterflyPip.shader = "FFTOCEAN_BUTTERFLY_COMP";
      butterflyPip.pipLayout.descSets.push_back( butterflySet );
      butterflyPip.pipLayout.ranges.push_back( oceanParamsRange );
   }

   // Inversion and permutation pipeline
   // ==============================================================================================
   {
      DescriptorSetLayoutInfo inversionPermutationSet = {};  // Set 0

      const ShaderResourceInfo displacementInfo = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 0};
      const ShaderResourceInfo pingpong0Info = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 1};
      const ShaderResourceInfo pingpong1Info = {
          ShaderResourceType::STORAGE_IMAGE, ShaderStage::COMPUTE_STAGE, 2};

      inversionPermutationSet.shaderResources.push_back( displacementInfo );
      inversionPermutationSet.shaderResources.push_back( pingpong0Info );
      inversionPermutationSet.shaderResources.push_back( pingpong1Info );

      inversionPermutationPip.shader = "FFTOCEAN_INVERSIONPERMUTATION_COMP";
      inversionPermutationPip.pipLayout.descSets.push_back( inversionPermutationSet );
      inversionPermutationPip.pipLayout.ranges.push_back( oceanParamsRange );
   }

   return true;
}
}
