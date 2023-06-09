#include <ECS/Systems/Procedural/FFTOceanSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/Components/Procedural/FFTOceanComponent.h>

#include <Algorithms/BitManipulation.h>

#include <cmath>
#include <numeric>

namespace CYD
{
enum class FourierComponent  // To compute the displacement for a specific component
{
   X,
   Y,
   Z
};

static void Initialize( FFTOceanComponent& ocean )
{
   ocean.butterflyOperationsPip  = StaticPipelines::FindByName( "BUTTERFLY_OPERATIONS" );
   ocean.inversionPermutationPip = StaticPipelines::FindByName( "INVERSION_PERMUTATION" );
   ocean.philipsSpectraGenPip = StaticPipelines::FindByName( "PHILLIPS_SPECTRA_GENERATION" );
   ocean.butterflyTexGenerationPip =
       StaticPipelines::FindByName( "BUTTERFLY_TEX_GENERATION" );
   ocean.fourierComponentsPip = StaticPipelines::FindByName( "FOURIER_COMPONENTS" );
}

static void computeDisplacement(
    CmdListHandle cmdList,
    const StaticMaterialComponent& /*material*/,
    FFTOceanComponent& ocean,  // Make this const?
    FourierComponent fourierComponent )
{
   const uint32_t resolution     = ocean.params.resolution;
   const uint32_t numberOfStages = static_cast<uint32_t>( std::log2( resolution ) );

   // Cooley-Tukey Radix-2 FFT GPU algorithm
   GRIS::BindPipeline( cmdList, ocean.butterflyOperationsPip );

   GRIS::BindImage( cmdList, ocean.butterflyTexture, 0, 0 );

   switch( fourierComponent )
   {
      case FourierComponent::X:
         GRIS::BindImage( cmdList, ocean.fourierComponentsX, 1, 0 );
         break;
      case FourierComponent::Y:
         GRIS::BindImage( cmdList, ocean.fourierComponentsY, 1, 0 );
         break;
      case FourierComponent::Z:
         GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 1, 0 );
         break;
   }

   GRIS::BindImage( cmdList, ocean.pingpongTex, 2, 0 );

   ocean.params.pingpong = 0;

   ocean.params.direction = 0;
   for( uint32_t i = 0; i < numberOfStages; ++i )
   {
      ocean.params.stage = i;

      GRIS::UpdateConstantBuffer(
          cmdList,
          PipelineStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::Parameters ),
          &ocean.params );

      GRIS::Dispatch( cmdList, resolution, resolution, 1 );

      // Horizontal butterfly shaderpass
      ocean.params.pingpong = !ocean.params.pingpong;
   }

   ocean.params.direction = 1;
   for( uint32_t i = 0; i < numberOfStages; ++i )
   {
      ocean.params.stage = i;

      GRIS::UpdateConstantBuffer(
          cmdList,
          PipelineStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::Parameters ),
          &ocean.params );

      GRIS::Dispatch( cmdList, resolution, resolution, 1 );

      // Vertical butterfly shaderpass
      ocean.params.pingpong = !ocean.params.pingpong;
   }

   // Inversion and permutation shaderpass
   GRIS::BindPipeline( cmdList, ocean.inversionPermutationPip );

   // GRIS::BindImage( cmdList, material., 0, 0 );

   switch( fourierComponent )
   {
      case FourierComponent::X:
         ocean.params.componentMask = 0x8;
         GRIS::BindImage( cmdList, ocean.fourierComponentsX, 1, 0 );
         break;
      case FourierComponent::Y:
         ocean.params.componentMask = 0x4;
         GRIS::BindImage( cmdList, ocean.fourierComponentsY, 1, 0 );
         break;
      case FourierComponent::Z:
         ocean.params.componentMask = 0x2;
         GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 1, 0 );
         break;
   }

   GRIS::BindImage( cmdList, ocean.pingpongTex, 2, 0 );

   GRIS::UpdateConstantBuffer(
       cmdList,
       PipelineStage::COMPUTE_STAGE,
       0,
       sizeof( FFTOceanComponent::Parameters ),
       &ocean.params );

   GRIS::Dispatch( cmdList, resolution, resolution, 1 );
}

void FFTOceanSystem::tick( double deltaS )
{
   for( const auto& entityEntry : m_entities )
   {
      StaticMaterialComponent& material = *std::get<StaticMaterialComponent*>( entityEntry.arch );
      FFTOceanComponent& ocean    = *std::get<FFTOceanComponent*>( entityEntry.arch );

      // First time run
      if( ocean.needsInit )
      {
         Initialize( ocean );
         ocean.needsInit = false;
      }

      // Updating time elapsed
      ocean.params.time += static_cast<float>( deltaS );

      const CmdListHandle cmdList = GRIS::CreateCommandList( COMPUTE );

      const uint32_t resolution     = ocean.params.resolution;
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

         // GRIS::DestroyTexture( material.data.disp );

         // TODO Size based on dimensions and pixel format
         TextureDescription rgTexDesc = {};
         rgTexDesc.size               = resolution * resolution * 2 * sizeof( float );
         rgTexDesc.width              = resolution;
         rgTexDesc.height             = resolution;
         rgTexDesc.type               = ImageType::TEXTURE_2D;
         rgTexDesc.format             = PixelFormat::RG32F;
         rgTexDesc.usage              = ImageUsage::STORAGE;
         rgTexDesc.stages             = PipelineStage::COMPUTE_STAGE;

         ocean.spectrum1 = GRIS::CreateTexture( rgTexDesc );
         ocean.spectrum2 = GRIS::CreateTexture( rgTexDesc );

         ocean.fourierComponentsY = GRIS::CreateTexture( rgTexDesc );
         ocean.fourierComponentsX = GRIS::CreateTexture( rgTexDesc );
         ocean.fourierComponentsZ = GRIS::CreateTexture( rgTexDesc );

         ocean.pingpongTex = GRIS::CreateTexture( rgTexDesc );

         TextureDescription dispTexDesc = {};
         dispTexDesc.size               = resolution * resolution * 4 * sizeof( float );
         dispTexDesc.width              = resolution;
         dispTexDesc.height             = resolution;
         dispTexDesc.type               = ImageType::TEXTURE_2D;
         dispTexDesc.format             = PixelFormat::RGBA32F;
         dispTexDesc.usage              = ImageUsage::STORAGE | ImageUsage::SAMPLED;
         dispTexDesc.stages = PipelineStage::COMPUTE_STAGE | PipelineStage::VERTEX_STAGE |
                              PipelineStage::FRAGMENT_STAGE;

         // material.data.disp = GRIS::CreateTexture( dispTexDesc );

         TextureDescription butterflyDesc = {};
         butterflyDesc.size               = resolution * numberOfStages * 4 * sizeof( float );
         butterflyDesc.width              = numberOfStages;
         butterflyDesc.height             = resolution;
         butterflyDesc.type               = ImageType::TEXTURE_2D;
         butterflyDesc.format             = PixelFormat::RGBA32F;
         butterflyDesc.usage              = ImageUsage::STORAGE;
         butterflyDesc.stages             = PipelineStage::COMPUTE_STAGE;

         ocean.butterflyTexture = GRIS::CreateTexture( butterflyDesc );

         ocean.bitReversedIndices = GRIS::CreateBuffer(
             sizeof( uint32_t ) * resolution, "FFTOceanSystem Bit-Reversed Indices" );

         ocean.resolutionChanged = false;

         // Since the resolution changed, we force an update of the pre-computed textures
         ocean.needsUpdate = true;
      }

      // Generating pre-computed textures (independent)
      // ===========================================================================================
      if( ocean.needsUpdate )
      {
         // Generate the two Phillips spectrum textures
         GRIS::BindPipeline( cmdList, ocean.philipsSpectraGenPip );

         GRIS::UpdateConstantBuffer(
             cmdList,
             PipelineStage::COMPUTE_STAGE,
             0,
             sizeof( FFTOceanComponent::Parameters ),
             &ocean.params );

         GRIS::BindImage( cmdList, ocean.spectrum1, 0, 0 );
         GRIS::BindImage( cmdList, ocean.spectrum2, 1, 0 );
         GRIS::Dispatch( cmdList, resolution, resolution, 1 );

         // Generating Butterfly texture
         std::vector<uint32_t> indices( resolution );  // Bit-reversed indices
         std::iota( indices.begin(), indices.end(), static_cast<uint32_t>( 0 ) );
         EMP::BitReversalPermutation( indices );

         const size_t indicesDataSize = indices.size() * sizeof( indices[0] );
         GRIS::CopyToBuffer( ocean.bitReversedIndices, indices.data(), 0, indicesDataSize );

         GRIS::BindPipeline( cmdList, ocean.butterflyTexGenerationPip );

         GRIS::UpdateConstantBuffer(
             cmdList,
             PipelineStage::COMPUTE_STAGE,
             0,
             sizeof( FFTOceanComponent::Parameters ),
             &ocean.params );

         GRIS::BindImage( cmdList, ocean.butterflyTexture, 0, 0 );
         GRIS::BindBuffer( cmdList, ocean.bitReversedIndices, 1, 0 );
         GRIS::Dispatch( cmdList, numberOfStages, resolution, 1 );

         ocean.needsUpdate = false;
      }

      // Generating time-dependent textures
      // ===========================================================================================

      // Generating Fourier components time-dependent textures (dy, dx, dz)
      GRIS::BindPipeline( cmdList, ocean.fourierComponentsPip );

      GRIS::UpdateConstantBuffer(
          cmdList,
          PipelineStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::Parameters ),
          &ocean.params );

      GRIS::BindImage( cmdList, ocean.fourierComponentsY, 0, 0 );
      GRIS::BindImage( cmdList, ocean.fourierComponentsX, 1, 0 );
      GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 2, 0 );
      GRIS::BindImage( cmdList, ocean.spectrum1, 3, 0 );
      GRIS::BindImage( cmdList, ocean.spectrum2, 4, 0 );
      GRIS::Dispatch( cmdList, resolution, resolution, 1 );

      computeDisplacement( cmdList, material, ocean, FourierComponent::X );
      computeDisplacement( cmdList, material, ocean, FourierComponent::Y );
      computeDisplacement( cmdList, material, ocean, FourierComponent::Z );

      RenderGraph::AddPass( cmdList );
   }
}
}