#include <Graphics/Vulkan/Device.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Instance.h>
#include <Graphics/Vulkan/Surface.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/PipelineStash.h>
#include <Graphics/Vulkan/RenderPassStash.h>
#include <Graphics/Vulkan/SamplerStash.h>
#include <Graphics/Vulkan/CommandPool.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/DescriptorPool.h>

#include <algorithm>

static constexpr float DEFAULT_PRIORITY            = 1.0f;
static constexpr uint32_t NUMBER_QUEUES_PER_FAMILY = 2;

// VK Resource Pools Sizes
static constexpr uint32_t MAX_BUFFER_COUNT  = 512;
static constexpr uint32_t MAX_TEXTURE_COUNT = 512;

namespace vk
{
Device::Device(
    const CYD::Window& window,
    const Instance& instance,
    const Surface& surface,
    const VkPhysicalDevice& physDevice,
    const std::vector<const char*>& extensions )
    : m_window( window ),
      m_instance( instance ),
      m_surface( surface ),
      m_extensions( extensions ),
      m_physDevice( physDevice )
{
   m_buffers.resize( MAX_BUFFER_COUNT );
   m_textures.resize( MAX_TEXTURE_COUNT );

   _populateQueueFamilies();
   _createLogicalDevice();
   _fetchQueues();
   _createCommandPools();
   _createDescriptorPool();

   m_renderPasses = std::make_unique<RenderPassStash>( *this );
   m_pipelines    = std::make_unique<PipelineStash>( *this );
   m_samplers     = std::make_unique<SamplerStash>( *this );
}

void Device::_populateQueueFamilies()
{
   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties( m_physDevice, &queueFamilyCount, nullptr );

   std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
   vkGetPhysicalDeviceQueueFamilyProperties(
       m_physDevice, &queueFamilyCount, queueFamilies.data() );

   for( uint32_t i = 0; i < queueFamilies.size(); ++i )
   {
      CYD::QueueUsageFlag type = 0;

      VkQueueFlags vkQueueType = queueFamilies[i].queueFlags;
      if( vkQueueType & VK_QUEUE_GRAPHICS_BIT )
      {
         type |= static_cast<uint32_t>( CYD::QueueUsage::GRAPHICS );
      }
      if( vkQueueType & VK_QUEUE_COMPUTE_BIT )
      {
         type |= static_cast<uint32_t>( CYD::QueueUsage::COMPUTE );
      }
      if( vkQueueType & VK_QUEUE_TRANSFER_BIT )
      {
         type |= static_cast<uint32_t>( CYD::QueueUsage::TRANSFER );
      }

      VkBool32 supportsPresent = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(
          m_physDevice, i, m_surface.getVKSurface(), &supportsPresent );

      m_queueFamilies.push_back(
          { {}, i, queueFamilies[i].queueCount, type, static_cast<bool>( supportsPresent ) } );
   }
}

void Device::_createLogicalDevice()
{
   // Populating queue infos
   std::vector<VkDeviceQueueCreateInfo> queueInfos;

   for( auto& family : m_queueFamilies )
   {
      family.queues.resize( NUMBER_QUEUES_PER_FAMILY );

      VkDeviceQueueCreateInfo queueInfo = {};
      queueInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.pNext                   = nullptr;
      queueInfo.flags                   = 0;
      queueInfo.queueFamilyIndex        = family.index;
      queueInfo.queueCount              = NUMBER_QUEUES_PER_FAMILY;
      queueInfo.pQueuePriorities        = &DEFAULT_PRIORITY;

      queueInfos.push_back( std::move( queueInfo ) );
   }

   // Create logical device
   VkPhysicalDeviceFeatures deviceFeatures = {};
   vkGetPhysicalDeviceFeatures( m_physDevice, &deviceFeatures );

   m_physProps = std::make_unique<VkPhysicalDeviceProperties>();
   vkGetPhysicalDeviceProperties( m_physDevice, m_physProps.get() );

   const std::vector<const char*> layers = m_instance.getLayers();

   VkDeviceCreateInfo deviceInfo      = {};
   deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   deviceInfo.pNext                   = nullptr;
   deviceInfo.flags                   = 0;
   deviceInfo.queueCreateInfoCount    = static_cast<uint32_t>( queueInfos.size() );
   deviceInfo.pQueueCreateInfos       = queueInfos.data();
   deviceInfo.enabledLayerCount       = static_cast<uint32_t>( layers.size() );
   deviceInfo.ppEnabledLayerNames     = layers.data();
   deviceInfo.enabledExtensionCount   = static_cast<uint32_t>( m_extensions.size() );
   deviceInfo.ppEnabledExtensionNames = m_extensions.data();
   deviceInfo.pEnabledFeatures        = &deviceFeatures;

   VkResult result = vkCreateDevice( m_physDevice, &deviceInfo, nullptr, &m_vkDevice );
   CYDASSERT( result == VK_SUCCESS && "Device: Could not create logical device" );
}

void Device::_fetchQueues()
{
   for( QueueFamily& queueFamily : m_queueFamilies )
   {
      auto& queues = queueFamily.queues;
      for( uint32_t i = 0; i < queues.size(); ++i )
      {
         vkGetDeviceQueue( m_vkDevice, queueFamily.index, i, &queues[i] );
      }
   }
}

void Device::_createCommandPools()
{
   for( const QueueFamily& queueFamily : m_queueFamilies )
   {
      m_commandPools.emplace_back( std::make_unique<CommandPool>(
          *this, queueFamily.index, queueFamily.type, queueFamily.supportsPresent ) );
   }
}

void Device::_createDescriptorPool() { m_descPool = std::make_unique<DescriptorPool>( *this ); }

// =================================================================================================
// Command buffers

CommandBuffer* Device::createCommandBuffer( CYD::QueueUsageFlag usage, bool presentable )
{
   // TODO Implement support for EXACT type so that we can possibly send work (like transfer for
   // example) to another queue family

   CommandBuffer* cmdBuffer = nullptr;

   // Attempting to find an adequate type
   const auto it = std::find_if(
       m_commandPools.begin(),
       m_commandPools.end(),
       [usage, presentable]( const std::unique_ptr<CommandPool>& pool ) {
          return ( usage & pool->getType() ) && !( presentable && !pool->supportsPresentation() );
       } );
   if( it != m_commandPools.end() )
   {
      // Found an adequate command pool
      cmdBuffer = ( *it )->createCommandBuffer( usage );
   }

   return cmdBuffer;
}

// =================================================================================================
// Device buffers

Buffer*
Device::_createBuffer( size_t size, CYD::BufferUsageFlag usage, CYD::MemoryTypeFlag memoryType )
{
   // Check to see if we have a free spot for a buffer.
   auto it = std::find_if(
       m_buffers.rbegin(), m_buffers.rend(), []( Buffer& buffer ) { return !buffer.inUse(); } );

   if( it != m_buffers.rend() )
   {
      // We found a buffer that can be replaced
      it->release();
      it->acquire( *this, size, usage, memoryType );
      return &*it;
   }

   CYDASSERT( !"Device: Too many buffers in flight" );
   return nullptr;
}

Buffer* Device::createVertexBuffer( size_t size )
{
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_DST | CYD::BufferUsage::VERTEX,
       CYD::MemoryType::DEVICE_LOCAL );
}

Buffer* Device::createIndexBuffer( size_t size )
{
   // TODO Support for uint16 and uint32
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_DST | CYD::BufferUsage::INDEX,
       CYD::MemoryType::DEVICE_LOCAL );
}

Buffer* Device::createStagingBuffer( size_t size )
{
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_SRC,
       CYD::MemoryType::HOST_VISIBLE | CYD::MemoryType::HOST_COHERENT );
}

Buffer* Device::createUniformBuffer( size_t size )
{
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_DST | CYD::BufferUsage::UNIFORM,
       CYD::MemoryType::HOST_VISIBLE | CYD::MemoryType::HOST_COHERENT );
}

Buffer* Device::createBuffer( size_t size )
{
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_DST | CYD::BufferUsage::STORAGE,
       CYD::MemoryType::HOST_VISIBLE | CYD::MemoryType::HOST_COHERENT );
}

Texture* Device::createTexture( const CYD::TextureDescription& desc )
{
   // Check to see if we have a free spot for a texture.
   auto it = std::find_if( m_textures.rbegin(), m_textures.rend(), []( Texture& texture ) {
      return !texture.inUse();
   } );

   if( it != m_textures.rend() )
   {
      // We found a texture that can be replaced
      it->release();
      it->acquire( *this, desc );

      return &*it;
   }

   CYDASSERT( !"Device: Too many textures in flight" );
   return nullptr;
}

// =================================================================================================
// Swapchain

Swapchain* Device::createSwapchain( const CYD::SwapchainInfo& scInfo )
{
   CYDASSERT( !m_swapchain.get() && "Device: Swapchain already created" );

   if( !m_swapchain.get() && supportsPresentation() )
   {
      m_swapchain = std::make_unique<Swapchain>( *this, m_surface, scInfo );
   }
   return m_swapchain.get();
}

// =================================================================================================
// Cleanup

void Device::cleanup()
{
   // Cleaning up textures
   for( auto& texture : m_textures )
   {
      if( texture.getVKImage() && !texture.inUse() )
      {
         texture.release();
      }
   }

   // Cleaning up device buffers
   for( auto& buffer : m_buffers )
   {
      if( buffer.getVKBuffer() && !buffer.inUse() )
      {
         buffer.release();
      }
   }
}

// =================================================================================================
// Getters

const VkQueue* Device::getQueueFromFamily( uint32_t familyIndex ) const
{
   const std::vector<VkQueue>& vkQueues = m_queueFamilies[familyIndex].queues;
   if( !vkQueues.empty() )
   {
      // TODO More dynamic queue returns, maybe based on currently used or an even distribution
      return &vkQueues[0];
   }

   return nullptr;
}

const VkQueue* Device::getQueueFromUsage( CYD::QueueUsageFlag usage, bool supportsPresentation )
    const
{
   const auto it = std::find_if(
       m_queueFamilies.begin(),
       m_queueFamilies.end(),
       [usage, supportsPresentation]( const QueueFamily& family ) {
          return ( family.type & usage ) && !( !family.supportsPresent == supportsPresentation );
       } );
   if( it != m_queueFamilies.end() )
   {
      return getQueueFromFamily( it->index );
   }

   return nullptr;
}

uint32_t Device::findMemoryType( uint32_t typeFilter, uint32_t properties ) const
{
   VkPhysicalDeviceMemoryProperties memProperties;
   vkGetPhysicalDeviceMemoryProperties( m_physDevice, &memProperties );

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

bool Device::supportsPresentation() const
{
   const auto it = std::find_if(
       m_queueFamilies.begin(), m_queueFamilies.end(), []( const QueueFamily& family ) {
          return family.supportsPresent;
       } );
   if( it != m_queueFamilies.end() )
   {
      return true;
   }
   return false;
}

Device::~Device()
{
   vkDeviceWaitIdle( m_vkDevice );

   for( auto& buffer : m_buffers )
   {
      buffer.release();
   }

   for( auto& texture : m_textures )
   {
      texture.release();
   }

   m_samplers.reset();
   m_pipelines.reset();
   m_renderPasses.reset();
   m_swapchain.reset();
   for( auto& commandPool : m_commandPools )
   {
      commandPool.reset();
   }
   m_descPool.reset();

   vkDestroyDevice( m_vkDevice, nullptr );
}
}
