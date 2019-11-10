#include <Core/Graphics/Vulkan/DescriptorPool.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/PipelineStash.h>

cyd::DescriptorPool::DescriptorPool( const Device& device ) : _device( device )
{
   VkDescriptorPoolSize poolSize = {};
   poolSize.type                 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSize.descriptorCount      = 32;

   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
   poolInfo.poolSizeCount              = 1;
   poolInfo.pPoolSizes                 = &poolSize;
   poolInfo.maxSets                    = 32;

   VkResult result =
       vkCreateDescriptorPool( _device.getVKDevice(), &poolInfo, nullptr, &_vkDescPool );

   CYDASSERT( result == VK_SUCCESS && "CommandPool: Could not create descriptor pool" );
}

VkDescriptorSet cyd::DescriptorPool::findOrAllocate(
    uint32_t binding,
    const DescriptorSetLayoutInfo& layout )
{
   const auto descSetIt = _descSets.find( layout );
   if( descSetIt != _descSets.end() )
   {
      return descSetIt->second;
   }

   const VkDescriptorSetLayout vkDescSetLayout = _device.getPipelineStash().findOrCreate( layout );

   VkDescriptorSetAllocateInfo allocInfo = {};
   allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool              = _vkDescPool;
   allocInfo.descriptorSetCount          = 1;
   allocInfo.pSetLayouts                 = &vkDescSetLayout;

   VkDescriptorSet vkDescSet;
   VkResult result = vkAllocateDescriptorSets( _device.getVKDevice(), &allocInfo, &vkDescSet );
   CYDASSERT( result == VK_SUCCESS && "DescriptorPool: Failed to solo allocate descriptor set" );

   _descSets.insert( { layout, vkDescSet } );
   return vkDescSet;
}

void cyd::DescriptorPool::free( const VkDescriptorSet& descSet )
{
   vkFreeDescriptorSets( _device.getVKDevice(), _vkDescPool, 1, &descSet );
}

cyd::DescriptorPool::~DescriptorPool()
{
   vkDestroyDescriptorPool( _device.getVKDevice(), _vkDescPool, nullptr );
}
