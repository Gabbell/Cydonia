#include <Graphics/Vulkan/Buffer.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/TypeConversions.h>

namespace vk
{
void Buffer::seize(
    const Device& device,
    size_t size,
    cyd::BufferUsageFlag usage,
    cyd::MemoryTypeFlag memoryType )
{
   _device     = &device;
   _size       = size;
   _memoryType = memoryType;
   _inUse      = true;

   VkBufferCreateInfo bufferInfo = {};
   bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size               = _size;

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

   VkResult result = vkCreateBuffer( _device->getVKDevice(), &bufferInfo, nullptr, &_vkBuffer );
   CYDASSERT( result == VK_SUCCESS && "Buffer: Could not create buffer" );

   _allocateMemory();
}

void Buffer::release()
{
   if( _device )
   {
      _size       = 0;
      _memoryType = 0;
      _inUse      = false;

      vkDestroyBuffer( _device->getVKDevice(), _vkBuffer, nullptr );
      vkFreeMemory( _device->getVKDevice(), _vkMemory, nullptr );

      _device    = nullptr;
      _data      = nullptr;
      _vkBuffer  = nullptr;
      _vkMemory  = nullptr;
      _vkDescSet = nullptr;
   }
}

void Buffer::mapMemory( const void* data )
{
   if( !( _memoryType & cyd::MemoryType::HOST_VISIBLE ) )
   {
      CYDASSERT( !"Buffer: Cannot map memory, buffer memory not host visible" );
      return;
   }

   // TODO Add offsets to create chunking buffers
   vkMapMemory( _device->getVKDevice(), _vkMemory, 0, _size, 0, &_data );
   memcpy( _data, data, _size );
   vkUnmapMemory( _device->getVKDevice(), _vkMemory );
}

void Buffer::updateDescriptorSet( const cyd::ShaderObjectInfo& info, VkDescriptorSet descSet )
{
   _vkDescSet = descSet;

   VkDescriptorBufferInfo bufferInfo = {};
   bufferInfo.buffer                 = _vkBuffer;
   bufferInfo.offset                 = 0;
   bufferInfo.range                  = _size;

   VkWriteDescriptorSet descriptorWrite = {};
   descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   descriptorWrite.dstSet               = _vkDescSet;
   descriptorWrite.dstBinding           = info.binding;
   descriptorWrite.dstArrayElement      = 0;
   descriptorWrite.descriptorType =
       TypeConversions::cydShaderObjectTypeToVkDescriptorType( info.type );
   descriptorWrite.descriptorCount = 1;
   descriptorWrite.pBufferInfo     = &bufferInfo;

   vkUpdateDescriptorSets( _device->getVKDevice(), 1, &descriptorWrite, 0, nullptr );
}

void Buffer::_allocateMemory()
{
   // Allocating memory
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements( _device->getVKDevice(), _vkBuffer, &memRequirements );

   VkMemoryPropertyFlags memoryProperty = 0;
   if( _memoryType & cyd::MemoryType::DEVICE_LOCAL )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
   }
   if( _memoryType & cyd::MemoryType::HOST_VISIBLE )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   }
   if( _memoryType & cyd::MemoryType::HOST_COHERENT )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
   }

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize       = memRequirements.size;
   allocInfo.memoryTypeIndex =
       _device->findMemoryType( memRequirements.memoryTypeBits, memoryProperty );

   VkResult result = vkAllocateMemory( _device->getVKDevice(), &allocInfo, nullptr, &_vkMemory );
   CYDASSERT( result == VK_SUCCESS && "Buffer: Could not allocate device memory" );

   vkBindBufferMemory( _device->getVKDevice(), _vkBuffer, _vkMemory, 0 );
}
}
