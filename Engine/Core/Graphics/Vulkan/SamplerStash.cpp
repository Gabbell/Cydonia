#include <Core/Graphics/Vulkan/SamplerStash.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Device.h>

cyd::SamplerStash::SamplerStash( const Device& device ) : _device( device ) {}

const VkSampler cyd::SamplerStash::findOrCreate( const SamplerInfo& info )
{
   const auto it = _samplers.find( info );
   if( it != _samplers.end() )
   {
      return it->second;
   }

   VkSamplerCreateInfo samplerInfo = {};
   samplerInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.magFilter           = cydFilterToVkFilter( info.magFilter );
   samplerInfo.minFilter           = cydFilterToVkFilter( info.minFilter );

   VkSamplerAddressMode addressMode    = cydAddressModeToVkAddressMode( info.addressMode );
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
   VkResult result = vkCreateSampler( _device.getVKDevice(), &samplerInfo, nullptr, &vkSampler );
   CYDASSERT( result == VK_SUCCESS && "SamplerStash: Could not create sampler" );

   _samplers.insert( { info, vkSampler } );

   return vkSampler;
}

cyd::SamplerStash::~SamplerStash()
{
   for( const auto& sampler : _samplers )
   {
      vkDestroySampler( _device.getVKDevice(), sampler.second, nullptr );
   }
}
