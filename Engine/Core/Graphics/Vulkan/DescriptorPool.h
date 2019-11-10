#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

#include <cstdint>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkDescriptorPool );
FWDHANDLE( VkDescriptorSet );
namespace cyd
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class DescriptorPool
{
  public:
   DescriptorPool( const Device& device );
   ~DescriptorPool();

   VkDescriptorSet findOrAllocate( uint32_t binding, const DescriptorSetLayoutInfo& layout );
   void free( const VkDescriptorSet& descSet );

  private:
   const Device& _device;

   std::unordered_map<DescriptorSetLayoutInfo, VkDescriptorSet> _descSets;

   VkDescriptorPool _vkDescPool;
};
}
