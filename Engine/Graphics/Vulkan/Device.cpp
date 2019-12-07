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
static constexpr uint32_t MAX_BUFFER_COUNT  = 64;
static constexpr uint32_t MAX_TEXTURE_COUNT = 64;

namespace vk
{
Device::Device(
    const cyd::Window& window,
    const Instance& instance,
    const Surface& surface,
    const VkPhysicalDevice& physDevice,
    const std::vector<const char*>& extensions )
    : _window( window ),
      _instance( instance ),
      _surface( surface ),
      _extensions( extensions ),
      _physDevice( physDevice )
{
   _buffers.resize( MAX_BUFFER_COUNT );
   _textures.resize( MAX_TEXTURE_COUNT );

   _populateQueueFamilies();
   _createLogicalDevice();
   _fetchQueues();
   _createCommandPools();
   _createDescriptorPool();

   _renderPasses = std::make_unique<RenderPassStash>( *this );
   _pipelines    = std::make_unique<PipelineStash>( *this );
   _samplers     = std::make_unique<SamplerStash>( *this );
}

void Device::_populateQueueFamilies()
{
   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties( _physDevice, &queueFamilyCount, nullptr );

   std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
   vkGetPhysicalDeviceQueueFamilyProperties( _physDevice, &queueFamilyCount, queueFamilies.data() );

   for( uint32_t i = 0; i < queueFamilies.size(); ++i )
   {
      cyd::QueueUsageFlag type = 0;

      VkQueueFlags vkQueueType = queueFamilies[i].queueFlags;
      if( vkQueueType & VK_QUEUE_GRAPHICS_BIT )
      {
         type |= static_cast<uint32_t>( cyd::QueueUsage::GRAPHICS );
      }
      if( vkQueueType & VK_QUEUE_COMPUTE_BIT )
      {
         type |= static_cast<uint32_t>( cyd::QueueUsage::COMPUTE );
      }
      if( vkQueueType & VK_QUEUE_TRANSFER_BIT )
      {
         type |= static_cast<uint32_t>( cyd::QueueUsage::TRANSFER );
      }

      VkBool32 supportsPresent = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(
          _physDevice, i, _surface.getVKSurface(), &supportsPresent );

      _queueFamilies.push_back(
          { {}, i, queueFamilies[i].queueCount, type, static_cast<bool>( supportsPresent ) } );
   }
}

void Device::_createLogicalDevice()
{
   // Populating queue infos
   std::vector<VkDeviceQueueCreateInfo> queueInfos;

   for( auto& family : _queueFamilies )
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
   vkGetPhysicalDeviceFeatures( _physDevice, &deviceFeatures );

   _physProps = std::make_unique<VkPhysicalDeviceProperties>();
   vkGetPhysicalDeviceProperties( _physDevice, _physProps.get() );

   const std::vector<const char*> layers = _instance.getLayers();

   VkDeviceCreateInfo deviceInfo      = {};
   deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   deviceInfo.pNext                   = nullptr;
   deviceInfo.flags                   = 0;
   deviceInfo.queueCreateInfoCount    = static_cast<uint32_t>( queueInfos.size() );
   deviceInfo.pQueueCreateInfos       = queueInfos.data();
   deviceInfo.enabledLayerCount       = static_cast<uint32_t>( layers.size() );
   deviceInfo.ppEnabledLayerNames     = layers.data();
   deviceInfo.enabledExtensionCount   = static_cast<uint32_t>( _extensions.size() );
   deviceInfo.ppEnabledExtensionNames = _extensions.data();
   deviceInfo.pEnabledFeatures        = &deviceFeatures;

   VkResult result = vkCreateDevice( _physDevice, &deviceInfo, nullptr, &_vkDevice );
   CYDASSERT( result == VK_SUCCESS && "VEDevice: Could not create logical device" );
}

void Device::_fetchQueues()
{
   for( QueueFamily& queueFamily : _queueFamilies )
   {
      auto& queues = queueFamily.queues;
      for( uint32_t i = 0; i < queues.size(); ++i )
      {
         vkGetDeviceQueue( _vkDevice, queueFamily.index, i, &queues[i] );
      }
   }
}

void Device::_createCommandPools()
{
   for( const QueueFamily& queueFamily : _queueFamilies )
   {
      _commandPools.push_back( std::make_unique<CommandPool>(
          *this, queueFamily.index, queueFamily.type, queueFamily.supportsPresent ) );
   }
}

void Device::_createDescriptorPool() { _descPool = std::make_unique<DescriptorPool>( *this ); }

// =================================================================================================
// Command buffers

CommandBuffer* Device::createCommandBuffer( cyd::QueueUsageFlag usage, bool presentable )
{
   // TODO Implement support for EXACT type so that we can possibly send work (like transfer for
   // example) to another queue family

   CommandBuffer* cmdBuffer = nullptr;

   // Attempting to find an adequate type
   const auto it = std::find_if(
       _commandPools.begin(),
       _commandPools.end(),
       [usage, presentable]( const std::unique_ptr<CommandPool>& pool ) {
          return ( usage & pool->getType() ) && !( presentable && !pool->supportsPresentation() );
       } );
   if( it != _commandPools.end() )
   {
      // Found an adequate command pool
      cmdBuffer = ( *it )->createCommandBuffer();
   }

   return cmdBuffer;
}

// =================================================================================================
// Device buffers

Buffer*
Device::_createBuffer( size_t size, cyd::BufferUsageFlag usage, cyd::MemoryTypeFlag memoryType )
{
   // Check to see if we have a free spot for a buffer.
   auto it = std::find_if(
       _buffers.rbegin(), _buffers.rend(), []( Buffer& buffer ) { return !buffer.inUse(); } );

   if( it != _buffers.rend() )
   {
      // We found a buffer that can be replaced
      it->release();
      it->seize( *this, size, usage, memoryType );
      return &*it;
   }

   CYDASSERT( !"Device: Too many buffers in flight" );
   return nullptr;
}

Buffer* Device::createVertexBuffer( size_t size )
{
   return _createBuffer(
       size,
       cyd::BufferUsage::TRANSFER_DST | cyd::BufferUsage::VERTEX,
       cyd::MemoryType::DEVICE_LOCAL );
}

Buffer* Device::createIndexBuffer( size_t size )
{
   // TODO Support for uint16 and uint32
   return _createBuffer(
       size,
       cyd::BufferUsage::TRANSFER_DST | cyd::BufferUsage::INDEX,
       cyd::MemoryType::DEVICE_LOCAL );
}

Buffer* Device::createUniformBuffer(
    size_t size,
    const cyd::ShaderObjectInfo& info,
    const cyd::DescriptorSetLayoutInfo& layout )
{
   Buffer* uniformBuffer = _createBuffer(
       size,
       cyd::BufferUsage::TRANSFER_DST | cyd::BufferUsage::UNIFORM,
       cyd::MemoryType::HOST_VISIBLE | cyd::MemoryType::HOST_COHERENT );

   VkDescriptorSet descSet = _descPool->findOrAllocate( layout );

   uniformBuffer->updateDescriptorSet( info, descSet );

   return uniformBuffer;
}

Buffer* Device::createStagingBuffer( size_t size )
{
   return _createBuffer(
       size,
       cyd::BufferUsage::TRANSFER_SRC,
       cyd::MemoryType::HOST_VISIBLE | cyd::MemoryType::HOST_COHERENT );
}

Texture* Device::createTexture(
    const cyd::TextureDescription& desc,
    const cyd::ShaderObjectInfo& info,
    const cyd::DescriptorSetLayoutInfo& layout )
{
   // Check to see if we have a free spot for a texture.
   auto it = std::find_if(
       _textures.rbegin(), _textures.rend(), []( Texture& texture ) { return !texture.inUse(); } );

   if( it != _textures.rend() )
   {
      // We found a texture that can be replaced
      it->release();
      it->seize( *this, desc );

      VkDescriptorSet descSet = _descPool->findOrAllocate( layout );

      it->updateDescriptorSet( info, descSet );

      return &*it;
   }

   CYDASSERT( !"Device: Too many textures in flight" );
   return nullptr;
}

// =================================================================================================
// Swapchain

Swapchain* Device::createSwapchain( const cyd::SwapchainInfo& scInfo )
{
   CYDASSERT( !_swapchain.get() && "Device: Swapchain already created" );

   if( !_swapchain.get() && supportsPresentation() )
   {
      _swapchain = std::make_unique<Swapchain>( *this, _surface, scInfo );
   }
   return _swapchain.get();
}

// =================================================================================================
// Cleanup

void Device::cleanup()
{
   // Cleaning up textures
   for( auto& texture : _textures )
   {
      if( !texture.inUse() )
      {
         // Freeing texture's assigned descriptor set
         _descPool->free( texture.getVKDescSet() );

         texture.release();
      }
   }

   // Cleaning up device buffers
   for( auto& buffer : _buffers )
   {
      if( !buffer.inUse() )
      {
         // Freeing buffer's assigned descriptor set
         _descPool->free( buffer.getVKDescSet() );

         buffer.release();
      }
   }
}

// =================================================================================================
// Getters

const VkQueue* Device::getQueueFromFamily( uint32_t familyIndex ) const
{
   const std::vector<VkQueue>& vkQueues = _queueFamilies[familyIndex].queues;
   if( !vkQueues.empty() )
   {
      // TODO More dynamic queue returns, maybe based on currently used or an even distribution
      return &vkQueues[0];
   }

   return nullptr;
}

const VkQueue* Device::getQueueFromUsage( cyd::QueueUsageFlag usage, bool supportsPresentation )
    const
{
   const auto it = std::find_if(
       _queueFamilies.begin(),
       _queueFamilies.end(),
       [usage, supportsPresentation]( const QueueFamily& family ) {
          return ( family.type & usage ) && !( !family.supportsPresent == supportsPresentation );
       } );
   if( it != _queueFamilies.end() )
   {
      return getQueueFromFamily( it->index );
   }

   return nullptr;
}

uint32_t Device::findMemoryType( uint32_t typeFilter, uint32_t properties ) const
{
   VkPhysicalDeviceMemoryProperties memProperties;
   vkGetPhysicalDeviceMemoryProperties( _physDevice, &memProperties );

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
   const auto it =
       std::find_if( _queueFamilies.begin(), _queueFamilies.end(), []( const QueueFamily& family ) {
          return family.supportsPresent;
       } );
   if( it != _queueFamilies.end() )
   {
      return true;
   }
   return false;
}

uint32_t Device::maxPushConstantsSize() const
{
   return _physProps ? _physProps->limits.maxPushConstantsSize : 0;
}

Device::~Device()
{
   vkDeviceWaitIdle( _vkDevice );

   for( auto& buffer : _buffers )
   {
      buffer.release();
   }

   for( auto& texture : _textures )
   {
      texture.release();
   }

   _samplers.reset();
   _pipelines.reset();
   _renderPasses.reset();
   _swapchain.reset();
   _descPool.reset();
   for( auto& commandPool : _commandPools )
   {
      commandPool.reset();
   }

   vkDestroyDevice( _vkDevice, nullptr );
}
}
