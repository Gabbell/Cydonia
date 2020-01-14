#include <Graphics/Vulkan/Buffer.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/TypeConversions.h>

namespace vk
{
void Buffer::acquire(
    const Device& device,
    size_t size,
    cyd::BufferUsageFlag usage,
    cyd::MemoryTypeFlag memoryType )
{
   m_pDevice    = &device;
   m_size       = size;
   m_memoryType = memoryType;
   m_inUse      = true;

   VkBufferCreateInfo bufferInfo = {};
   bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size               = m_size;

   if( usage & cyd::BufferUsage::UNIFORM )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
   }
   if( usage & cyd::BufferUsage::STORAGE )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
   }
   if( usage & cyd::BufferUsage::TRANSFER_SRC )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
   }
   if( usage & cyd::BufferUsage::TRANSFER_DST )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   }
   if( usage & cyd::BufferUsage::INDEX )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
   }
   if( usage & cyd::BufferUsage::VERTEX )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   }

   // TODO Investigate concurrent
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VkResult result = vkCreateBuffer( m_pDevice->getVKDevice(), &bufferInfo, nullptr, &m_vkBuffer );
   CYDASSERT( result == VK_SUCCESS && "Buffer: Could not create buffer" );

   _allocateMemory();
}

void Buffer::release()
{
   if( m_pDevice )
   {
      m_size       = 0;
      m_memoryType = 0;
      m_inUse      = false;

      vkDestroyBuffer( m_pDevice->getVKDevice(), m_vkBuffer, nullptr );
      vkFreeMemory( m_pDevice->getVKDevice(), m_vkMemory, nullptr );

      m_pDevice   = nullptr;
      m_data      = nullptr;
      m_vkBuffer  = nullptr;
      m_vkMemory  = nullptr;
      m_vkDescSet = nullptr;
   }
}

void Buffer::mapMemory( const void* pData )
{
   if( !( m_memoryType & cyd::MemoryType::HOST_VISIBLE ) )
   {
      CYDASSERT( !"Buffer: Cannot map memory, buffer memory not host visible" );
      return;
   }

   // TODO Add offsets to create chunking buffers
   vkMapMemory( m_pDevice->getVKDevice(), m_vkMemory, 0, m_size, 0, &m_data );
   memcpy( m_data, pData, m_size );
   vkUnmapMemory( m_pDevice->getVKDevice(), m_vkMemory );
}

void Buffer::updateDescriptorSet( const cyd::ShaderObjectInfo& info, VkDescriptorSet descSet )
{
   m_vkDescSet = descSet;

   VkDescriptorBufferInfo bufferInfo = {};
   bufferInfo.buffer                 = m_vkBuffer;
   bufferInfo.offset                 = 0;
   bufferInfo.range                  = m_size;

   VkWriteDescriptorSet descriptorWrite = {};
   descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrite.dstSet               = m_vkDescSet;
   descriptorWrite.dstBinding           = info.binding;
   descriptorWrite.dstArrayElement      = 0;
   descriptorWrite.descriptorType =
       TypeConversions::cydShaderObjectTypeToVkDescriptorType( info.type );
   descriptorWrite.descriptorCount = 1;
   descriptorWrite.pBufferInfo     = &bufferInfo;

   vkUpdateDescriptorSets( m_pDevice->getVKDevice(), 1, &descriptorWrite, 0, nullptr );
}

void Buffer::_allocateMemory()
{
   // Allocating memory
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements( m_pDevice->getVKDevice(), m_vkBuffer, &memRequirements );

   VkMemoryPropertyFlags memoryProperty = 0;
   if( m_memoryType & cyd::MemoryType::DEVICE_LOCAL )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
   }
   if( m_memoryType & cyd::MemoryType::HOST_VISIBLE )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   }
   if( m_memoryType & cyd::MemoryType::HOST_COHERENT )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
   }

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize       = memRequirements.size;
   allocInfo.memoryTypeIndex =
       m_pDevice->findMemoryType( memRequirements.memoryTypeBits, memoryProperty );

   VkResult result = vkAllocateMemory( m_pDevice->getVKDevice(), &allocInfo, nullptr, &m_vkMemory );
   CYDASSERT( result == VK_SUCCESS && "Buffer: Could not allocate device memory" );

   vkBindBufferMemory( m_pDevice->getVKDevice(), m_vkBuffer, m_vkMemory, 0 );
}
}
