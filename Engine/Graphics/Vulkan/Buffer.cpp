#include <Graphics/Vulkan/Buffer.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>

namespace vk
{
Buffer::Buffer() { m_useCount = std::make_unique<std::atomic<uint32_t>>( 0 ); }

void Buffer::incUse() { ( *m_useCount )++; }
void Buffer::decUse()
{
   if( m_useCount->load() == 0 )
   {
      CYDASSERT( !"Buffer: Decrementing use count would go below 0" );
      return;
   }

   ( *m_useCount )--;
}

void Buffer::acquire(
    const Device& device,
    size_t size,
    CYD::BufferUsageFlag usage,
    CYD::MemoryTypeFlag memoryType,
    const std::string_view name )
{
   m_name       = name;
   m_pDevice    = &device;
   m_size       = size;
   m_memoryType = memoryType;

   CYDASSERT(
       ( m_useCount->load() ) == 0 &&
       "Buffer: Use count was not 0. This buffer was probably not released" );

   VkBufferCreateInfo bufferInfo = {};
   bufferInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size               = m_size;

   if( usage & CYD::BufferUsage::UNIFORM )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
   }
   if( usage & CYD::BufferUsage::STORAGE )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
   }
   if( usage & CYD::BufferUsage::TRANSFER_SRC )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
   }
   if( usage & CYD::BufferUsage::TRANSFER_DST )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
   }
   if( usage & CYD::BufferUsage::INDEX )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
   }
   if( usage & CYD::BufferUsage::VERTEX )
   {
      bufferInfo.usage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   }

   // TODO Investigate concurrent
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   VkResult result = vkCreateBuffer( m_pDevice->getVKDevice(), &bufferInfo, nullptr, &m_vkBuffer );
   CYDASSERT( result == VK_SUCCESS && "Buffer: Could not create buffer" );

   _allocateMemory();

   incUse();
}

void Buffer::release()
{
   if( m_pDevice )
   {
      CYDASSERT( m_useCount->load() == 0 && "Buffer: released a still used buffer" );

      vkDestroyBuffer( m_pDevice->getVKDevice(), m_vkBuffer, nullptr );
      vkFreeMemory( m_pDevice->getVKDevice(), m_vkMemory, nullptr );

      m_name       = DEFAULT_BUFFER_NAME;
      m_size       = 0;
      m_memoryType = 0;
      m_pDevice    = nullptr;
      m_vkBuffer   = nullptr;
      m_vkMemory   = nullptr;
   }
}

void Buffer::copy( const void* pData, size_t offset, size_t size )
{
   if( !( m_memoryType & CYD::MemoryType::HOST_VISIBLE ) )
   {
      CYDASSERT( !"Buffer: Cannot copy to buffer, buffer memory not host visible" );
      return;
   }

   if( ( offset + size ) > m_size )
   {
      CYDASSERT( !"Buffer: Offset + size surpasses allocated buffer size" );
      return;
   }

   _mapMemory();
   memcpy( static_cast<unsigned char*>( m_data ) + offset, pData, size );
   _unmapMemory();
}

void Buffer::_mapMemory()
{
   if( !( m_memoryType & CYD::MemoryType::HOST_VISIBLE ) )
   {
      CYDASSERT( !"Buffer: Cannot map memory, buffer memory not host visible" );
      return;
   }

   // TODO Add offsets to create chunking buffers
   const VkResult result =
       vkMapMemory( m_pDevice->getVKDevice(), m_vkMemory, 0, m_size, 0, &m_data );
   CYDASSERT( result == VK_SUCCESS && "Buffer: Mapping memory failed" );
}

void Buffer::_unmapMemory()
{
   if( !( m_memoryType & CYD::MemoryType::HOST_VISIBLE ) )
   {
      CYDASSERT( !"Buffer: Cannot unmap memory, buffer memory not host visible" );
      return;
   }

   if( m_data )
   {
      vkUnmapMemory( m_pDevice->getVKDevice(), m_vkMemory );
      m_data = nullptr;
   }
}

void Buffer::_allocateMemory()
{
   // Allocating memory
   VkMemoryRequirements memRequirements;
   vkGetBufferMemoryRequirements( m_pDevice->getVKDevice(), m_vkBuffer, &memRequirements );

   VkMemoryPropertyFlags memoryProperty = 0;
   if( m_memoryType & CYD::MemoryType::DEVICE_LOCAL )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
   }
   if( m_memoryType & CYD::MemoryType::HOST_VISIBLE )
   {
      memoryProperty |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   }
   if( m_memoryType & CYD::MemoryType::HOST_COHERENT )
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

Buffer::~Buffer() { release(); }
}
