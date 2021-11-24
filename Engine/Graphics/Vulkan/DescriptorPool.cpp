#include <Graphics/Vulkan/DescriptorPool.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/PipelineCache.h>

#include <array>

namespace vk
{
DescriptorPool::DescriptorPool( const Device& device ) : m_device( device )
{
   std::array<VkDescriptorPoolSize, 5> poolSizes = {};

   const auto& limits = m_device.getProperties()->limits;

   const uint32_t maxDescriptorSetUniformBuffers =
       std::min( 128u, limits.maxDescriptorSetUniformBuffers );
   const uint32_t maxDescriptorSetStorageBuffers =
       std::min( 128u, limits.maxDescriptorSetStorageBuffers );
   const uint32_t maxDescriptorSetSampledImages =
       std::min( 128u, limits.maxDescriptorSetSampledImages );
   const uint32_t maxDescriptorSetStorageImages =
       std::min( 128u, limits.maxDescriptorSetStorageImages );
   const uint32_t maxDescriptorSetSamplers = std::min( 128u, limits.maxDescriptorSetSamplers );

   const uint32_t totalDescriptorSets =
       maxDescriptorSetUniformBuffers + maxDescriptorSetStorageBuffers +
       maxDescriptorSetSampledImages + maxDescriptorSetStorageImages + maxDescriptorSetSamplers;

   // TODO Make more descriptor pools based on different types?
   poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   poolSizes[0].descriptorCount = maxDescriptorSetUniformBuffers;

   poolSizes[1].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
   poolSizes[1].descriptorCount = maxDescriptorSetStorageBuffers;

   poolSizes[2].type            = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
   poolSizes[2].descriptorCount = maxDescriptorSetSamplers;

   poolSizes[3].type            = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
   poolSizes[3].descriptorCount = maxDescriptorSetStorageImages;

   poolSizes[4].type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
   poolSizes[4].descriptorCount = maxDescriptorSetSampledImages;

   VkDescriptorPoolCreateInfo poolInfo = {};
   poolInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   poolInfo.flags                      = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
   poolInfo.poolSizeCount              = static_cast<uint32_t>( poolSizes.size() );
   poolInfo.pPoolSizes                 = poolSizes.data();
   poolInfo.maxSets                    = totalDescriptorSets;

   VkResult result =
       vkCreateDescriptorPool( m_device.getVKDevice(), &poolInfo, nullptr, &m_vkDescPool );

   CYDASSERT( result == VK_SUCCESS && "DescriptorPool: Could not create descriptor pool" );
}

VkDescriptorSet DescriptorPool::allocate( const CYD::ShaderSetInfo& layout ) const
{
   const VkDescriptorSetLayout vkDescSetLayout = m_device.getPipelineCache().findOrCreate( layout );

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

void DescriptorPool::free( const VkDescriptorSet* shaderSets, const uint32_t count ) const
{
   vkFreeDescriptorSets( m_device.getVKDevice(), m_vkDescPool, count, shaderSets );
}

DescriptorPool::~DescriptorPool()
{
   vkDestroyDescriptorPool( m_device.getVKDevice(), m_vkDescPool, nullptr );
}
}
