#include <Graphics/Vulkan/DescriptorPool.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/PipelineStash.h>

#include <array>

namespace vk
{
DescriptorPool::DescriptorPool( const Device& device ) : m_device( device )
{
   std::array<VkDescriptorPoolSize, 2> poolSizes = {};

   poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSizes[0].descriptorCount = 32;

   poolSizes[1].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   poolSizes[1].descriptorCount = 32;

   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
   poolInfo.poolSizeCount              = static_cast<uint32_t>( poolSizes.size() );
   poolInfo.pPoolSizes                 = poolSizes.data();
   poolInfo.maxSets                    = 32;

   VkResult result =
       vkCreateDescriptorPool( m_device.getVKDevice(), &poolInfo, nullptr, &m_vkDescPool );

   CYDASSERT( result == VK_SUCCESS && "DescriptorPool: Could not create descriptor pool" );
}

VkDescriptorSet DescriptorPool::allocate( const cyd::DescriptorSetLayoutInfo& layout ) const
{
   const VkDescriptorSetLayout vkDescSetLayout = m_device.getPipelineStash().findOrCreate( layout );

   VkDescriptorSetAllocateInfo allocInfo = {};
   allocInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   allocInfo.descriptorPool              = m_vkDescPool;
   allocInfo.descriptorSetCount          = 1;
   allocInfo.pSetLayouts                 = &vkDescSetLayout;

   VkDescriptorSet vkDescSet;
   VkResult result = vkAllocateDescriptorSets( m_device.getVKDevice(), &allocInfo, &vkDescSet );
   CYDASSERT( result == VK_SUCCESS && "DescriptorPool: Failed to solo allocate descriptor set" );

   return vkDescSet;
}

void DescriptorPool::free( const VkDescriptorSet& descSet ) const
{
   vkFreeDescriptorSets( m_device.getVKDevice(), m_vkDescPool, 1, &descSet );
}

DescriptorPool::~DescriptorPool()
{
   vkDestroyDescriptorPool( m_device.getVKDevice(), m_vkDescPool, nullptr );
}
}
