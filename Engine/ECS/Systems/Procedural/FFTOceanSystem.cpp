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
static PipelineIndex s_normalsJacobianPip      = INVALID_PIPELINE_IDX;

enum class Axis  // To compute the displacement for a specific axis
{
   X,
   Y,
   Z
};

static void Initialize()
{
   s_butterflyOperationsPip  = StaticPipelines::FindByName( "BUTTERFLY_OPERATIONS" );
   s_inversionPermutationPip = StaticPipelines::FindByName( "INVERSION_PERMUTATION" );
   s_philipsSpectraGenPip    = StaticPipelines::FindByName( "PHILLIPS_SPECTRUM_GENERATION" );
   s_butterflyTexGenPip      = StaticPipelines::FindByName( "BUTTERFLY_TEX_GENERATION" );
   s_fourierComponentsPip    = StaticPipelines::FindByName( "FOURIER_COMPONENTS" );
   s_normalsJacobianPip      = StaticPipelines::FindByName( "NORMALS_JACOBIAN" );
   s_initialized             = true;
}

static void GeneratePhillipsSpectrum( CmdListHandle cmdList, FFTOceanComponent& ocean )
{
   // Generate the two Phillips spectrum textures
   CYD_SCOPED_GPUTRACE( cmdList, "Generate Philips Spectra" );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   const uint32_t groupsX =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeX ) );
   const uint32_t groupsY =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeY ) );

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
}

static void
GenerateButterflyTexture( CmdListHandle cmdList, FFTOceanComponent& ocean, uint32_t numberOfStages )
{
   // Generating Butterfly texture
   CYD_SCOPED_GPUTRACE( cmdList, "Generate Butterfly Texture" );

   constexpr float localSizeY = 16.0f;
   const uint32_t groupsY =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeY ) );

   std::vector<uint32_t> indices( ocean.params.resolution );  // Bit-reversed indices
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
}

static void GenerateFourierComponents( CmdListHandle cmdList, FFTOceanComponent& ocean )
{
   // Generating Fourier components time-dependent textures (dy, dx, dz)
   CYD_SCOPED_GPUTRACE( cmdList, "Generate Fourier Components Textures" );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   const uint32_t groupsX =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeX ) );
   const uint32_t groupsY =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeY ) );

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
}

static void ComputeDisplacement(
    CmdListHandle cmdList,
    FFTOceanComponent& ocean,  // Make this const?
    Axis axis,
    uint32_t numberOfStages )
{
   // Cooley-Tukey Radix-2 FFT GPU algorithm
   CYD_GPUTRACE_BEGIN( cmdList, "Butterfly Operations" );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   const uint32_t groupsX =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeX ) );
   const uint32_t groupsY =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeY ) );

   GRIS::BindPipeline( cmdList, s_butterflyOperationsPip );

   GRIS::BindImage( cmdList, ocean.butterflyTexture, 0 );

   switch( axis )
   {
      case Axis::X:
         GRIS::BindImage( cmdList, ocean.fourierComponentsX, 1 );
         break;
      case Axis::Y:
         GRIS::BindImage( cmdList, ocean.fourierComponentsY, 1 );
         break;
      case Axis::Z:
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

   switch( axis )
   {
      case Axis::X:
         ocean.params.componentMask = 0x4;
         GRIS::BindImage( cmdList, ocean.fourierComponentsX, 1 );
         break;
      case Axis::Y:
         ocean.params.componentMask = 0x2;
         GRIS::BindImage( cmdList, ocean.fourierComponentsY, 1 );
         break;
      case Axis::Z:
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

static void ComputeNormalsAndJacobian( CmdListHandle cmdList, FFTOceanComponent& ocean )
{
   // Calculating Normals and Jacobian (Fold map)
   CYD_SCOPED_GPUTRACE( cmdList, "Computing Normals & Jacobian" );

   constexpr float localSizeX = 16.0f;
   constexpr float localSizeY = 16.0f;
   const uint32_t groupsX =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeX ) );
   const uint32_t groupsY =
       static_cast<uint32_t>( std::ceil( ocean.params.resolution / localSizeY ) );

   GRIS::BindPipeline( cmdList, s_normalsJacobianPip );

   GRIS::UpdateConstantBuffer(
       cmdList,
       PipelineStage::COMPUTE_STAGE,
       0,
       sizeof( FFTOceanComponent::ShaderParameters ),
       &ocean.params );

   SamplerInfo sampler;
   sampler.addressMode = AddressMode::REPEAT;
   GRIS::BindTexture( cmdList, ocean.displacementMap, sampler, 0 );
   GRIS::BindImage( cmdList, ocean.normalMap, 1 );
   GRIS::Dispatch( cmdList, groupsX, groupsY, 1 );
}

void FFTOceanSystem::tick( double deltaS )
{
   CYD_TRACE();

   // First time run
   if( !s_initialized )
   {
      Initialize();
   }

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::PRE_RENDER );
   CYD_SCOPED_GPUTRACE( cmdList, "FFTOceanSystem" );

   for( const auto& entityEntry : m_entities )
   {
      MaterialComponent& material = GetComponent<MaterialComponent>( entityEntry );
      FFTOceanComponent& ocean    = GetComponent<FFTOceanComponent>( entityEntry );

      // Updating time elapsed
      ocean.params.time += static_cast<float>( deltaS );

      const uint32_t numberOfStages = static_cast<uint32_t>( std::log2( ocean.params.resolution ) );

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
         rgTexDesc.width              = ocean.params.resolution;
         rgTexDesc.height             = ocean.params.resolution;
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
         butterflyDesc.height             = ocean.params.resolution;
         butterflyDesc.type               = ImageType::TEXTURE_2D;
         butterflyDesc.format             = PixelFormat::RGBA32F;
         butterflyDesc.usage              = ImageUsage::STORAGE;
         butterflyDesc.stages             = PipelineStage::COMPUTE_STAGE;

         ocean.butterflyTexture = GRIS::CreateTexture( butterflyDesc );

         ocean.bitReversedIndices = GRIS::CreateBuffer(
             sizeof( uint32_t ) * ocean.params.resolution, "FFTOceanSystem Bit-Reversed Indices" );

         TextureDescription dispTexDesc = {};
         dispTexDesc.width              = ocean.params.resolution;
         dispTexDesc.height             = ocean.params.resolution;
         dispTexDesc.type               = ImageType::TEXTURE_2D;
         dispTexDesc.format             = PixelFormat::RGBA32F;
         dispTexDesc.usage              = ImageUsage::STORAGE | ImageUsage::SAMPLED;
         dispTexDesc.stages = PipelineStage::COMPUTE_STAGE | PipelineStage::VERTEX_STAGE |
                              PipelineStage::FRAGMENT_STAGE;
         dispTexDesc.name = "FFTOcean Displacement Map";

         ocean.displacementMap = GRIS::CreateTexture( dispTexDesc );

         TextureDescription normalTexDesc = {};
         normalTexDesc.width              = ocean.params.resolution;
         normalTexDesc.height             = ocean.params.resolution;
         normalTexDesc.type               = ImageType::TEXTURE_2D;
         normalTexDesc.format             = PixelFormat::RGBA32F;
         normalTexDesc.usage              = ImageUsage::STORAGE | ImageUsage::SAMPLED;
         normalTexDesc.stages = PipelineStage::COMPUTE_STAGE | PipelineStage::FRAGMENT_STAGE;
         normalTexDesc.name   = "FFTOcean Normal+Fold Map";

         ocean.normalMap = GRIS::CreateTexture( normalTexDesc );

         m_materials.updateMaterial(
             material.materialIdx,
             MaterialCache::TextureSlot::DISPLACEMENT,
             ocean.displacementMap );

         m_materials.updateMaterial(
             material.materialIdx, MaterialCache::TextureSlot::NORMAL, ocean.normalMap );

         ocean.resolutionChanged = false;

         // Since the resolution changed, we force an update of the pre-computed textures
         ocean.needsUpdate = true;
      }

      // Generating pre-computed textures (time-independent)
      // ===========================================================================================
      if( ocean.needsUpdate )
      {
         GeneratePhillipsSpectrum( cmdList, ocean );
         GenerateButterflyTexture( cmdList, ocean, numberOfStages );

         ocean.needsUpdate = false;
      }

      // Generating time-dependent textures
      // ===========================================================================================

      GenerateFourierComponents( cmdList, ocean );

      // Compute the XYZ displacements
      ComputeDisplacement( cmdList, ocean, Axis::X, numberOfStages );
      ComputeDisplacement( cmdList, ocean, Axis::Y, numberOfStages );
      ComputeDisplacement( cmdList, ocean, Axis::Z, numberOfStages );

      ComputeNormalsAndJacobian( cmdList, ocean );
   }
}
}