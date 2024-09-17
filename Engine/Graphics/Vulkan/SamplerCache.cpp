#include <Graphics/Vulkan/SamplerCache.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/TypeConversions.h>

namespace vk
{
SamplerCache::SamplerCache( const Device& device ) : m_device( device ) {}

const VkSampler SamplerCache::findOrCreate( const CYD::SamplerInfo& info )
{
   const auto it = m_samplers.find( info );
   if( it != m_samplers.end() )
   {
      return it->second;
   }

   const VkSamplerAddressMode addressMode = TypeConversions::cydToVkAddressMode( info.addressMode );

   VkSamplerCreateInfo samplerInfo;
   samplerInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.pNext                   = nullptr;
   samplerInfo.flags                   = 0;
   samplerInfo.magFilter               = TypeConversions::cydToVkFilter( info.magFilter );
   samplerInfo.minFilter               = TypeConversions::cydToVkFilter( info.minFilter );
   samplerInfo.addressModeU            = addressMode;
   samplerInfo.addressModeV            = addressMode;
   samplerInfo.addressModeW            = addressMode;
   samplerInfo.anisotropyEnable        = info.maxAnisotropy > 0.999f;
   samplerInfo.maxAnisotropy           = info.maxAnisotropy;
   samplerInfo.borderColor             = TypeConversions::cydToVkBorderColor( info.borderColor );
   samplerInfo.unnormalizedCoordinates = VK_FALSE;
   samplerInfo.compareEnable           = info.useCompare;
   samplerInfo.compareOp               = TypeConversions::cydToVkCompareOp( info.compare );
   samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
   samplerInfo.minLod                  = info.minLod;
   samplerInfo.maxLod                  = info.maxLod;
   samplerInfo.mipLodBias              = 0.0f;

   VkSampler vkSampler;
   VkResult result = vkCreateSampler( m_device.getVKDevice(), &samplerInfo, nullptr, &vkSampler );
   CYD_ASSERT( result == VK_SUCCESS && "SamplerCache: Could not create sampler" );

   return m_samplers.insert( { info, vkSampler } ).first->second;
}

SamplerCache::~SamplerCache()
{
   for( const auto& sampler : m_samplers )
   {
      vkDestroySampler( m_device.getVKDevice(), sampler.second, nullptr );
   }
}
}