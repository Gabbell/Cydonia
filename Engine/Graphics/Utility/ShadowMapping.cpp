#include <Graphics/Utility/ShadowMapping.h>

namespace CYD
{
static SamplerInfo GetPCFSampler( bool hasLinearFilter )
{
   SamplerInfo sampler;
   sampler.useCompare    = true;  // For PCF
   sampler.minFilter     = hasLinearFilter ? Filter::LINEAR : Filter::NEAREST;
   sampler.magFilter     = hasLinearFilter ? Filter::LINEAR : Filter::NEAREST;
   sampler.compare       = CompareOperator::LESS_EQUAL;
   sampler.addressMode   = AddressMode::CLAMP_TO_BORDER;
   sampler.borderColor   = BorderColor::OPAQUE_BLACK;
   sampler.minLod        = 0.0f;
   sampler.maxLod        = 0.0f;
   sampler.maxAnisotropy = 0.0f;
   return sampler;
}

static SamplerInfo GetPrefilteredSampler()
{
   SamplerInfo sampler;
   sampler.useCompare    = false;
   sampler.minFilter     = Filter::NEAREST;
   sampler.magFilter     = Filter::NEAREST;
   sampler.addressMode   = AddressMode::CLAMP_TO_BORDER;
   sampler.borderColor   = BorderColor::OPAQUE_BLACK;
   sampler.minLod        = 0.0f;
   sampler.maxLod        = 0.0f;
   sampler.maxAnisotropy = 0.0f;
   return sampler;
}

SamplerInfo ShadowMapping::GetSampler()
{
   if( false )
   {
      return GetPrefilteredSampler();
   }

   return GetPCFSampler( true );
}

TextureDescription ShadowMapping::GetFilterableTextureDescription(
    uint32_t resolution,
    uint32_t numCascades )
{
   TextureDescription texDesc;
   texDesc.width  = resolution;
   texDesc.height = resolution;
   texDesc.depth  = numCascades;
   texDesc.type   = ImageType::TEXTURE_2D_ARRAY;
   texDesc.format = PixelFormat::R32F;
   texDesc.usage  = ImageUsage::SAMPLED | ImageUsage::COLOR | ImageUsage::TRANSFER_SRC |
                   ImageUsage::TRANSFER_DST;
   texDesc.stages = PipelineStage::FRAGMENT_STAGE;
   texDesc.name   = "Shadow Map Color";

   return texDesc;
}

TextureDescription ShadowMapping::GetDepthTextureDescription(
    uint32_t resolution,
    uint32_t numCascades )
{
   TextureDescription texDesc;
   texDesc.width  = resolution;
   texDesc.height = resolution;
   texDesc.depth  = numCascades;
   texDesc.type   = ImageType::TEXTURE_2D_ARRAY;
   texDesc.format = PixelFormat::D32_SFLOAT;
   texDesc.usage  = ImageUsage::SAMPLED | ImageUsage::DEPTH_STENCIL;
   texDesc.stages = PipelineStage::FRAGMENT_STAGE;
   texDesc.name   = "Shadow Map Depth";

   return texDesc;
}
}