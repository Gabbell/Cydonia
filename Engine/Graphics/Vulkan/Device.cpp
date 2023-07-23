#include <Graphics/Vulkan/Device.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/Instance.h>
#include <Graphics/Vulkan/Surface.h>
#include <Graphics/Vulkan/Swapchain.h>
#include <Graphics/Vulkan/PipelineCache.h>
#include <Graphics/Vulkan/RenderPassCache.h>
#include <Graphics/Vulkan/SamplerCache.h>
#include <Graphics/Vulkan/CommandPoolManager.h>
#include <Graphics/Vulkan/CommandBufferPool.h>
#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/Texture.h>
#include <Graphics/Vulkan/DescriptorPool.h>

#include <algorithm>

static constexpr float DEFAULT_PRIORITY            = 1.0f;
static constexpr uint32_t NUMBER_QUEUES_PER_FAMILY = 1;

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

   m_descPool           = std::make_unique<DescriptorPool>( *this );
   m_commandPoolManager = std::make_unique<CommandPoolManager>(
       *this, static_cast<uint32_t>( m_queueFamilies.size() ), 3 );
   m_renderPasses = std::make_unique<RenderPassCache>( *this );
   m_pipelines    = std::make_unique<PipelineCache>( *this );
   m_samplers     = std::make_unique<SamplerCache>( *this );
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
          { {}, queueFamilies[i].queueCount, type, static_cast<bool>( supportsPresent ) } );
   }
}

void Device::_createLogicalDevice()
{
   // Populating queue infos
   std::vector<VkDeviceQueueCreateInfo> queueInfos;

   for( uint32_t familyIdx = 0; familyIdx < m_queueFamilies.size(); ++familyIdx )
   {
      m_queueFamilies[familyIdx].queues.resize( NUMBER_QUEUES_PER_FAMILY );

      VkDeviceQueueCreateInfo queueInfo = {};
      queueInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.pNext                   = nullptr;
      queueInfo.flags                   = 0;
      queueInfo.queueFamilyIndex        = familyIdx;
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
   CYD_ASSERT( result == VK_SUCCESS && "Device: Could not create logical device" );
}

void Device::_fetchQueues()
{
   for( uint32_t familyIdx = 0; familyIdx < m_queueFamilies.size(); ++familyIdx )
   {
      auto& queues = m_queueFamilies[familyIdx].queues;
      for( uint32_t queueIdx = 0; queueIdx < queues.size(); ++queueIdx )
      {
         vkGetDeviceQueue( m_vkDevice, familyIdx, queueIdx, &queues[queueIdx] );
      }
   }
}

// =================================================================================================
// Command buffers

CommandBuffer* Device::createCommandBuffer(
    CYD::QueueUsageFlag usage,
    const std::string_view name,
    bool presentable )
{
   return m_commandPoolManager->acquire( usage, name, presentable, m_swapchain->getCurrentFrame() );
}

// =================================================================================================
// Device buffers

Buffer* Device::_createBuffer(
    size_t size,
    CYD::BufferUsageFlag usage,
    CYD::MemoryTypeFlag memoryType,
    const std::string_view name )
{
   // Check to see if we have a free spot for a buffer.
   auto it = std::find_if(
       m_buffers.rbegin(), m_buffers.rend(), []( Buffer& buffer ) { return !buffer.inUse(); } );

   if( it != m_buffers.rend() )
   {
      // We found a buffer that can be replaced
      it->release();
      it->acquire( *this, size, usage, memoryType, name );
      return &*it;
   }

   CYD_ASSERT( !"Device: Too many buffers in flight" );
   return nullptr;
}

Buffer* Device::createVertexBuffer( size_t size, const std::string_view name )
{
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_DST | CYD::BufferUsage::VERTEX,
       CYD::MemoryType::DEVICE_LOCAL,
       name );
}

Buffer* Device::createIndexBuffer( size_t size, const std::string_view name )
{
   // TODO Support for uint16 and uint32
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_DST | CYD::BufferUsage::INDEX,
       CYD::MemoryType::DEVICE_LOCAL,
       name );
}

Buffer* Device::createStagingBuffer( size_t size )
{
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_SRC,
       CYD::MemoryType::HOST_VISIBLE | CYD::MemoryType::HOST_COHERENT,
       "Staging Buffer" );
}

Buffer* Device::createUniformBuffer( size_t size, const std::string_view name )
{
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_DST | CYD::BufferUsage::UNIFORM,
       CYD::MemoryType::HOST_VISIBLE | CYD::MemoryType::HOST_COHERENT,
       name );
}

Buffer* Device::createBuffer( size_t size, const std::string_view name )
{
   return _createBuffer(
       size,
       CYD::BufferUsage::TRANSFER_DST | CYD::BufferUsage::STORAGE,
       CYD::MemoryType::HOST_VISIBLE | CYD::MemoryType::HOST_COHERENT,
       name );
}

Texture* Device::createTexture( const CYD::TextureDescription& desc )
{
   // Check to see if we have a free spot for a texture.
   auto it = std::find_if(
       m_textures.rbegin(),
       m_textures.rend(),
       []( Texture& texture ) { return !texture.inUse(); } );

   if( it != m_textures.rend() )
   {
      // We found a texture that can be replaced
      it->release();
      it->acquire( *this, desc );

      return &*it;
   }

   CYD_ASSERT( !"Device: Too many textures in flight" );
   return nullptr;
}

// =================================================================================================
// Swapchain

Swapchain* Device::createSwapchain( const CYD::SwapchainInfo& scInfo )
{
   CYD_ASSERT( !m_swapchain.get() && "Device: Swapchain already created" );

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
   m_commandPoolManager->cleanup();

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

void Device::clearPipelines() { m_pipelines->clear(); }

// =================================================================================================
// Synchronization

void Device::waitUntilIdle() { vkDeviceWaitIdle( m_vkDevice ); }
void Device::waitOnFrame( uint32_t currentFrame )
{
   m_commandPoolManager->waitOnFrame( currentFrame );
}

// =================================================================================================
// Getters

const Device::QueueFamily& Device::getQueueFamilyFromIndex( uint32_t familyIndex ) const
{
   CYD_ASSERT( familyIndex < m_queueFamilies.size() );
   return m_queueFamilies[familyIndex];
}

uint32_t Device::getQueueFamilyIndexFromUsage(
    CYD::QueueUsageFlag usage,
    bool supportsPresentation ) const
{
   for( uint32_t familyIdx = 0; familyIdx < m_queueFamilies.size(); ++familyIdx )
   {
      const QueueFamily& family = getQueueFamilyFromIndex( familyIdx );
      if( ( family.type & usage ) && !( !family.supportsPresent == supportsPresentation ) )
      {
         return familyIdx;
      }
   }

   return 0;
}

VkQueue Device::getQueueFromUsage( CYD::QueueUsageFlag usage, bool supportsPresentation ) const
{
   for( uint32_t familyIdx = 0; familyIdx < m_queueFamilies.size(); ++familyIdx )
   {
      const QueueFamily& family = getQueueFamilyFromIndex( familyIdx );

      if( ( family.type & usage ) && !( !family.supportsPresent == supportsPresentation ) &&
          !family.queues.empty() )
      {
         // TODO More dynamic queue returns, maybe based on currently used or an even
         // distribution
         return family.queues[0];
      }
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
   for( uint32_t familyIdx = 0; familyIdx < m_queueFamilies.size(); ++familyIdx )
   {
      if( m_queueFamilies[familyIdx].supportsPresent )
      {
         return true;
      }
   }
   return false;
}

Device::~Device()
{
   vkDeviceWaitIdle( m_vkDevice );

   cleanup();

   // Checking for any leaking resources
   CYD_ASSERT(
       std::find_if(
           m_buffers.begin(),
           m_buffers.end(),
           []( const Buffer& buffer ) { return buffer.getVKBuffer(); } ) == m_buffers.end() &&
       "Device: Some buffers are leaking" );
   CYD_ASSERT(
       std::find_if(
           m_textures.begin(),
           m_textures.end(),
           []( const Texture& texture ) { return texture.getVKImage(); } ) == m_textures.end() &&
       "Device: Some textures are leaking" );

   m_buffers.clear();
   m_textures.clear();

   m_samplers.reset();
   m_pipelines.reset();
   m_renderPasses.reset();
   m_swapchain.reset();
   m_commandPoolManager.reset();
   m_descPool.reset();

   vkDestroyDevice( m_vkDevice, nullptr );
}
}
