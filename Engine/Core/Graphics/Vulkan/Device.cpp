#include <Core/Graphics/Vulkan/Device.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Instance.h>
#include <Core/Graphics/Vulkan/Surface.h>
#include <Core/Graphics/Vulkan/Swapchain.h>
#include <Core/Graphics/Vulkan/PipelineStash.h>
#include <Core/Graphics/Vulkan/RenderPassStash.h>
#include <Core/Graphics/Vulkan/SamplerStash.h>
#include <Core/Graphics/Vulkan/CommandPool.h>
#include <Core/Graphics/Vulkan/Buffer.h>
#include <Core/Graphics/Vulkan/Texture.h>
#include <Core/Graphics/Vulkan/DescriptorPool.h>

#include <algorithm>

static constexpr float DEFAULT_PRIORITY            = 1.0f;
static constexpr uint32_t NUMBER_QUEUES_PER_FAMILY = 2;

cyd::Device::Device(
    const Instance& instance,
    const Window& window,
    const Surface& surface,
    const VkPhysicalDevice& physDevice,
    const std::vector<const char*>& extensions )
    : _instance( instance ),
      _window( window ),
      _surface( surface ),
      _extensions( extensions ),
      _physDevice( physDevice )
{
   _populateQueueFamilies();
   _createLogicalDevice();
   _fetchQueues();
   _createCommandPools();
   _createDescriptorPool();

   _renderPasses = std::make_unique<RenderPassStash>( *this );
   _pipelines    = std::make_unique<PipelineStash>( *this );
   _samplers     = std::make_unique<SamplerStash>( *this );
}

void cyd::Device::_populateQueueFamilies()
{
   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties( _physDevice, &queueFamilyCount, nullptr );

   std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
   vkGetPhysicalDeviceQueueFamilyProperties( _physDevice, &queueFamilyCount, queueFamilies.data() );

   for( uint32_t i = 0; i < queueFamilies.size(); ++i )
   {
      QueueUsageFlag type = 0;

      VkQueueFlags vkQueueType = queueFamilies[i].queueFlags;
      if( vkQueueType & VK_QUEUE_GRAPHICS_BIT )
      {
         type |= static_cast<uint32_t>( QueueUsage::GRAPHICS );
      }
      if( vkQueueType & VK_QUEUE_COMPUTE_BIT )
      {
         type |= static_cast<uint32_t>( QueueUsage::COMPUTE );
      }
      if( vkQueueType & VK_QUEUE_TRANSFER_BIT )
      {
         type |= static_cast<uint32_t>( QueueUsage::TRANSFER );
      }

      VkBool32 supportsPresent = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(
          _physDevice, i, _surface.getVKSurface(), &supportsPresent );

      _queueFamilies.push_back(
          { {}, i, queueFamilies[i].queueCount, type, static_cast<bool>( supportsPresent ) } );
   }
}

void cyd::Device::_createLogicalDevice()
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

void cyd::Device::_fetchQueues()
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

void cyd::Device::_createCommandPools()
{
   for( const QueueFamily& queueFamily : _queueFamilies )
   {
      _commandPools.push_back( std::make_unique<CommandPool>(
          *this, queueFamily.index, queueFamily.type, queueFamily.supportsPresent ) );
   }
}

void cyd::Device::_createDescriptorPool() { _descPool = std::make_unique<DescriptorPool>( *this ); }

std::shared_ptr<cyd::CommandBuffer> cyd::Device::createCommandBuffer(
    QueueUsageFlag usage,
    bool presentable )
{
   // TODO Implement support for EXACT type so that we can possibly send work (like transfer for
   // example) to another queue family

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
      return ( *it )->createCommandBuffer();
   }

   return nullptr;
}

std::shared_ptr<cyd::Buffer> cyd::Device::createDeviceBuffer( size_t size, BufferUsageFlag usage )
{
   return _buffers.emplace_back(
       std::make_shared<Buffer>( *this, size, usage, MemoryType::DEVICE_LOCAL ) );
}

std::shared_ptr<cyd::Buffer> cyd::Device::createUniformBuffer(
    BufferUsageFlag usage,
    const ShaderObjectInfo& info,
    const DescriptorSetLayoutInfo& layout )
{
   std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(
       *this,
       info.size,
       usage | BufferUsage::UNIFORM,
       MemoryType::HOST_VISIBLE | MemoryType::HOST_COHERENT );

   VkDescriptorSet descSet = _descPool->findOrAllocate( layout );

   buffer->updateDescriptorSet( info, descSet );

   return _buffers.emplace_back( buffer );
}

std::shared_ptr<cyd::Buffer> cyd::Device::createStagingBuffer( size_t size )
{
   return _buffers.emplace_back( std::make_shared<Buffer>(
       *this,
       size,
       BufferUsage::TRANSFER_SRC,
       MemoryType::HOST_VISIBLE | MemoryType::HOST_COHERENT ) );
}

std::shared_ptr<cyd::Texture> cyd::Device::createTexture(
    const TextureDescription& desc,
    const ShaderObjectInfo& info,
    const DescriptorSetLayoutInfo& layout )
{
   std::shared_ptr<Texture> texture = std::make_shared<Texture>( *this, desc );

   VkDescriptorSet descSet = _descPool->findOrAllocate( layout );

   texture->updateDescriptorSet( info, descSet );

   return _textures.emplace_back( texture );
}

cyd::Swapchain* cyd::Device::createSwapchain( const SwapchainInfo& scInfo )
{
   CYDASSERT( !_swapchain.get() && "Device: Swapchain already created" );

   if( !_swapchain.get() && supportsPresentation() )
   {
      _swapchain = std::make_unique<Swapchain>( *this, _surface, scInfo );
   }
   return _swapchain.get();
}

void cyd::Device::cleanup()
{
   // Cleaning up command buffers
   for( auto& commandPool : _commandPools )
   {
      commandPool->cleanup();
   }

   // Cleaning up textures
   for( auto& texture : _textures )
   {
      if( texture.use_count() == 1 )
      {
         // Freeing texture's assigned descriptor set
         _descPool->free( texture->getVKDescSet() );

         // Destroying texture
         texture.reset();
      }
   }

   // Cleaning up device buffers
   for( auto& buffer : _buffers )
   {
      if( buffer.use_count() == 1 )
      {
         // Freeing buffer's assigned descriptor set
         _descPool->free( buffer->getVKDescSet() );

         // Destroying buffer
         buffer.reset();
      }
   }
}

const VkQueue* cyd::Device::getQueueFromFamily( uint32_t familyIndex ) const
{
   const std::vector<VkQueue>& vkQueues = _queueFamilies[familyIndex].queues;
   if( !vkQueues.empty() )
   {
      // TODO More dynamic queue returns, maybe based on currently used or an even distribution
      return &vkQueues[0];
   }

   return nullptr;
}

const VkQueue* cyd::Device::getQueueFromUsage( QueueUsageFlag usage, bool supportsPresentation )
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

uint32_t cyd::Device::findMemoryType( uint32_t typeFilter, uint32_t properties ) const
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

bool cyd::Device::supportsPresentation() const
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

uint32_t cyd::Device::maxPushConstantsSize() const
{
   return _physProps ? _physProps->limits.maxPushConstantsSize : 0;
}

cyd::Device::~Device()
{
   vkDeviceWaitIdle( _vkDevice );

   for( auto& buffer : _buffers )
   {
      buffer.reset();
   }
   for( auto& texture : _textures )
   {
      texture.reset();
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
