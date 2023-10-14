#include <ECS/Systems/Procedural/FFTOceanSystem.h>

#include <Graphics/PipelineInfos.h>
#include <Graphics/StaticPipelines.h>
#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Scene/MaterialCache.h>

#include <ECS/Components/Procedural/FFTOceanComponent.h>

#include <Algorithms/BitManipulation.h>

#include <Profiling.h>

#include <cmath>
#include <numeric>

namespace CYD
{
static bool s_initialized                      = false;
static PipelineIndex s_butterflyOperationsPip  = INVALID_PIPELINE_IDX;
static PipelineIndex s_inversionPermutationPip = INVALID_PIPELINE_IDX;
static PipelineIndex s_philipsSpectraGenPip    = INVALID_PIPELINE_IDX;
static PipelineIndex s_butterflyTexGenPip      = INVALID_PIPELINE_IDX;
static PipelineIndex s_fourierComponentsPip    = INVALID_PIPELINE_IDX;
static PipelineIndex s_jacobianPip             = INVALID_PIPELINE_IDX;

enum class FourierComponent  // To compute the displacement for a specific component
{
   X,
   Y,
   Z
};

static void Initialize()
{
   s_butterflyOperationsPip  = StaticPipelines::FindByName( "BUTTERFLY_OPERATIONS" );
   s_inversionPermutationPip = StaticPipelines::FindByName( "INVERSION_PERMUTATION" );
   s_philipsSpectraGenPip    = StaticPipelines::FindByName( "PHILLIPS_SPECTRA_GENERATION" );
   s_butterflyTexGenPip      = StaticPipelines::FindByName( "BUTTERFLY_TEX_GENERATION" );
   s_fourierComponentsPip    = StaticPipelines::FindByName( "FOURIER_COMPONENTS" );
   s_jacobianPip             = StaticPipelines::FindByName( "JACOBIAN_FOLDMAP" );
   s_initialized             = true;
}

static void computeDisplacement(
    CmdListHandle cmdList,
    FFTOceanComponent& ocean,  // Make this const?
    FourierComponent fourierComponent,
    uint32_t numberOfStages,
    uint32_t groupsX,
    uint32_t groupsY )
{
   // Cooley-Tukey Radix-2 FFT GPU algorithm
   CYD_GPUTRACE_BEGIN( cmdList, "Butterfly Operations" );

   GRIS::BindPipeline( cmdList, s_butterflyOperationsPip );

   GRIS::BindImage( cmdList, ocean.butterflyTexture, 0 );

   switch( fourierComponent )
   {
      case FourierComponent::X:
         GRIS::BindImage( cmdList, ocean.fourierComponentsX, 1 );
         break;
      case FourierComponent::Y:
         GRIS::BindImage( cmdList, ocean.fourierComponentsY, 1 );
         break;
      case FourierComponent::Z:
         GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 1 );
         break;
   }

   GRIS::BindImage( cmdList, ocean.pingpongTex, 2 );

   ocean.params.pingpong = 0;

   ocean.params.direction = 0;
   for( uint32_t i = 0; i < numberOfStages; ++i )
   {
      ocean.params.stage = i;

      GRIS::UpdateConstantBuffer(
          cmdList,
          PipelineStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::ShaderParameters ),
          &ocean.params );

      GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

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
          sizeof( FFTOceanComponent::ShaderParameters ),
          &ocean.params );

      GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

      // Vertical butterfly shaderpass
      ocean.params.pingpong = !ocean.params.pingpong;
   }

   CYD_GPUTRACE_END( cmdList );

   // Inversion and permutation shaderpass
   CYD_GPUTRACE_BEGIN( cmdList, "Inversions Permutations" );

   GRIS::BindPipeline( cmdList, s_inversionPermutationPip );

   GRIS::BindImage( cmdList, ocean.displacementMap, 0 );

   switch( fourierComponent )
   {
      case FourierComponent::X:
         ocean.params.componentMask = 0x4;
         GRIS::BindImage( cmdList, ocean.fourierComponentsX, 1 );
         break;
      case FourierComponent::Y:
         ocean.params.componentMask = 0x2;
         GRIS::BindImage( cmdList, ocean.fourierComponentsY, 1 );
         break;
      case FourierComponent::Z:
         ocean.params.componentMask = 0x1;
         GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 1 );
         break;
   }

   GRIS::BindImage( cmdList, ocean.pingpongTex, 2 );

   GRIS::UpdateConstantBuffer(
       cmdList,
       PipelineStage::COMPUTE_STAGE,
       0,
       sizeof( FFTOceanComponent::ShaderParameters ),
       &ocean.params );

   GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

   CYD_GPUTRACE_END( cmdList );
}

void FFTOceanSystem::tick( double deltaS )
{
   CYD_TRACE( "FFTOceanSystem" );

   // First time run
   if( !s_initialized )
   {
      Initialize();
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER );
   CYD_SCOPED_GPUTRACE( cmdList, "FFTOceanSystem" );

   for( const auto& entityEntry : m_entities )
   {
      MaterialComponent& material = *std::get<MaterialComponent*>( entityEntry.arch );
      FFTOceanComponent& ocean    = *std::get<FFTOceanComponent*>( entityEntry.arch );

      // Updating time elapsed
      ocean.params.time += static_cast<float>( deltaS );

      const uint32_t resolution     = ocean.params.resolution;
      const uint32_t numberOfStages = static_cast<uint32_t>( std::log2( resolution ) );

      constexpr float localSizeX = 16.0f;
      constexpr float localSizeY = 16.0f;
      uint32_t groupsX           = static_cast<uint32_t>( std::ceil( resolution / localSizeX ) );
      uint32_t groupsY           = static_cast<uint32_t>( std::ceil( resolution / localSizeY ) );

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

         GRIS::DestroyTexture( ocean.displacementMap );

         // TODO Size based on dimensions and pixel format
         TextureDescription rgTexDesc = {};
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

         TextureDescription butterflyDesc = {};
         butterflyDesc.width              = numberOfStages;
         butterflyDesc.height             = resolution;
         butterflyDesc.type               = ImageType::TEXTURE_2D;
         butterflyDesc.format             = PixelFormat::RGBA32F;
         butterflyDesc.usage              = ImageUsage::STORAGE;
         butterflyDesc.stages             = PipelineStage::COMPUTE_STAGE;

         ocean.butterflyTexture = GRIS::CreateTexture( butterflyDesc );

         ocean.bitReversedIndices = GRIS::CreateBuffer(
             sizeof( uint32_t ) * resolution, "FFTOceanSystem Bit-Reversed Indices" );

         TextureDescription dispTexDesc = {};
         dispTexDesc.width              = resolution;
         dispTexDesc.height             = resolution;
         dispTexDesc.type               = ImageType::TEXTURE_2D;
         dispTexDesc.format             = PixelFormat::RGBA32F;
         dispTexDesc.usage              = ImageUsage::STORAGE | ImageUsage::SAMPLED;
         dispTexDesc.stages = PipelineStage::COMPUTE_STAGE | PipelineStage::VERTEX_STAGE |
                              PipelineStage::FRAGMENT_STAGE;

         ocean.displacementMap = GRIS::CreateTexture( dispTexDesc );

         m_materials.updateMaterial(
             material.materialIdx, ocean.displacementMap, static_cast<Material::TextureSlot>( 5 ) );

         ocean.resolutionChanged = false;

         // Since the resolution changed, we force an update of the pre-computed textures
         ocean.needsUpdate = true;
      }

      // Generating pre-computed textures (time-independent)
      // ===========================================================================================
      if( ocean.needsUpdate )
      {
         // Generate the two Phillips spectrum textures
         CYD_GPUTRACE_BEGIN( cmdList, "Generate Philips Spectra" );

         GRIS::BindPipeline( cmdList, s_philipsSpectraGenPip );

         GRIS::UpdateConstantBuffer(
             cmdList,
             PipelineStage::COMPUTE_STAGE,
             0,
             sizeof( FFTOceanComponent::ShaderParameters ),
             &ocean.params );

         GRIS::BindImage( cmdList, ocean.spectrum1, 0, 0 );
         GRIS::BindImage( cmdList, ocean.spectrum2, 1, 0 );
         GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

         CYD_GPUTRACE_END( cmdList );

         // Generating Butterfly texture
         CYD_GPUTRACE_BEGIN( cmdList, "Generate Butterfly Texture" );

         std::vector<uint32_t> indices( resolution );  // Bit-reversed indices
         std::iota( indices.begin(), indices.end(), static_cast<uint32_t>( 0 ) );
         EMP::BitReversalPermutation( indices );

         const UploadToBufferInfo info = { 0, indices.size() * sizeof( indices[0] ) };
         GRIS::UploadToBuffer( ocean.bitReversedIndices, indices.data(), info );

         GRIS::BindPipeline( cmdList, s_butterflyTexGenPip );

         GRIS::UpdateConstantBuffer(
             cmdList,
             PipelineStage::COMPUTE_STAGE,
             0,
             sizeof( FFTOceanComponent::ShaderParameters ),
             &ocean.params );

         GRIS::BindImage( cmdList, ocean.butterflyTexture, 0, 0 );
         GRIS::BindBuffer( cmdList, ocean.bitReversedIndices, 1, 0 );
         GRIS::Dispatch( cmdList, numberOfStages, groupsY, 1 );

         CYD_GPUTRACE_END( cmdList );

         ocean.needsUpdate = false;
      }

      // Generating time-dependent textures
      // ===========================================================================================

      // Generating Fourier components time-dependent textures (dy, dx, dz)
      CYD_GPUTRACE_BEGIN( cmdList, "Generate Fourier Components Textures" );

      GRIS::BindPipeline( cmdList, s_fourierComponentsPip );

      GRIS::UpdateConstantBuffer(
          cmdList,
          PipelineStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::ShaderParameters ),
          &ocean.params );

      GRIS::BindImage( cmdList, ocean.fourierComponentsY, 0 );
      GRIS::BindImage( cmdList, ocean.fourierComponentsX, 1 );
      GRIS::BindImage( cmdList, ocean.fourierComponentsZ, 2 );
      GRIS::BindImage( cmdList, ocean.spectrum1, 3 );
      GRIS::BindImage( cmdList, ocean.spectrum2, 4 );
      GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

      CYD_GPUTRACE_END( cmdList );

      computeDisplacement( cmdList, ocean, FourierComponent::X, numberOfStages, groupsX, groupsY );
      computeDisplacement( cmdList, ocean, FourierComponent::Y, numberOfStages, groupsX, groupsY );
      computeDisplacement( cmdList, ocean, FourierComponent::Z, numberOfStages, groupsX, groupsY );

      // Calculating Jacobian (Fold map)
      CYD_GPUTRACE_BEGIN( cmdList, "Computing Jacobian (Fold Map)" );

      GRIS::BindPipeline( cmdList, s_jacobianPip );

      GRIS::UpdateConstantBuffer(
          cmdList,
          PipelineStage::COMPUTE_STAGE,
          0,
          sizeof( FFTOceanComponent::ShaderParameters ),
          &ocean.params );

      GRIS::BindImage( cmdList, ocean.displacementMap, 0 );
      GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );

      CYD_GPUTRACE_END( cmdList );
   }
}
}