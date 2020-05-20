#include <Graphics/Vulkan/SamplerStash.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/TypeConversions.h>

namespace vk
{
SamplerStash::SamplerStash( const Device& device ) : m_device( device ) {}

const VkSampler SamplerStash::findOrCreate( const CYD::SamplerInfo& info )
{
   const auto it = m_samplers.find( info );
   if( it != m_samplers.end() )
   {
      return it->second;
   }

   VkSamplerCreateInfo samplerInfo = {};
   samplerInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.magFilter           = TypeConversions::cydToVkFilter( info.magFilter );
   samplerInfo.minFilter           = TypeConversions::cydToVkFilter( info.minFilter );

   VkSamplerAddressMode addressMode =
       TypeConversions::cydToVkAddressMode( info.addressMode );
   samplerInfo.addressModeU            = addressMode;
   samplerInfo.addressModeV            = addressMode;
   samplerInfo.addressModeW            = addressMode;
   samplerInfo.anisotropyEnable        = info.useAnisotropy;
   samplerInfo.maxAnisotropy           = info.maxAnisotropy;
   samplerInfo.borderColor             = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
   samplerInfo.unnormalizedCoordinates = VK_FALSE;
   samplerInfo.compareEnable           = VK_FALSE;
   samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
   samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;

   VkSampler vkSampler;
   VkResult result = vkCreateSampler( m_device.getVKDevice(), &samplerInfo, nullptr, &vkSampler );
   CYDASSERT( result == VK_SUCCESS && "SamplerStash: Could not create sampler" );

   return m_samplers.insert( {info, vkSampler} ).first->second;
}

SamplerStash::~SamplerStash()
{
   for( const auto& sampler : m_samplers )
   {
      vkDestroySampler( m_device.getVKDevice(), sampler.second, nullptr );
   }
}
}