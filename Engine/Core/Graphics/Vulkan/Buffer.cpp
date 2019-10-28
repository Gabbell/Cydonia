#include <Core/Graphics/Vulkan/Buffer.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Device.h>

cyd::Buffer::Buffer(
    const Device& device,
    size_t size,
    BufferUsageFlag usage,
    MemoryTypeFlag memoryType )
    : _device( device ), _size( size ), _usage( usage ), _memoryType( memoryType )
{
   VkBufferCreateInfo bufferInfo = {};
   bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size               = _size;

   if( usage & BufferUsage::TRANSFER_SRC )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
   }
   if( usage & BufferUsage::TRANSFER_DST )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   }
   if( usage & BufferUsage::UNIFORM )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
   }
   if( usage & BufferUsage::STORAGE )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
   }
   if( usage & BufferUsage::INDEX )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
   }
   if( usage & BufferUsage::VERTEX )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   }

   // TODO Investigate concurrent
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;


   VkResult result = vkCreateBuffer( _device.getVKDevice(), &bufferInfo, nullptr, &_vkBuffer );
   CYDASSERT( result == VK_SUCCESS && "Buffer: Could not create buffer" );

   _allocateMemory();
}

void cyd::Buffer::mapMemory( void* data, size_t size )
{
   // TODO Add offsets to create chunking buffers
   vkMapMemory( _device.getVKDevice(), _vkMemory, 0, size, 0, &_data );
   memcpy( _data, data, size );
   vkUnmapMemory( _device.getVKDevice(), _vkMemory );
}

static uint32_t findMemoryType(
    const VkPhysicalDevice& physicalDevice,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties )
{
   VkPhysicalDeviceMemoryProperties memProperties;
   vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );

   for( uint32_t i = 0; i < memProperties.memoryTypeCount; i++ )
   {
      if( ( typeFilter & ( 1 << i ) ) &&
          ( memProperties.memoryTypes[i].propertyFlags & properties ) == properties )
      {
         return i;
      }
   }

   return 0;
}

void cyd::Buffer::_allocateMemory()
{
   // Allocating memory
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements( _device.getVKDevice(), _vkBuffer, &memRequirements );

   VkMemoryPropertyFlags memoryProperty = 0;
   if( _memoryType & MemoryType::DEVICE_LOCAL )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
   }
   if( _memoryType & MemoryType::HOST_VISIBLE )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   }
   if( _memoryType & MemoryType::HOST_COHERENT )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
   }

   VkMemoryAllocateInfo allocInfo = {};
   allocInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
   allocInfo.allocationSize       = memRequirements.size;
   allocInfo.memoryTypeIndex      = findMemoryType(
       _device.getPhysicalDevice(), memRequirements.memoryTypeBits, memoryProperty );

   VkResult result = vkAllocateMemory( _device.getVKDevice(), &allocInfo, nullptr, &_vkMemory );
   CYDASSERT( result == VK_SUCCESS && "Buffer: Could not allocate device memory" );

   vkBindBufferMemory( _device.getVKDevice(), _vkBuffer, _vkMemory, 0 );
}

cyd::Buffer::~Buffer()
{
   vkDestroyBuffer( _device.getVKDevice(), _vkBuffer, nullptr );
   vkFreeMemory( _device.getVKDevice(), _vkMemory, nullptr );
}
