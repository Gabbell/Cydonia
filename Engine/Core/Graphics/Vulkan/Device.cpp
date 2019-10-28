#include <Core/Graphics/Vulkan/Device.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Instance.h>
#include <Core/Graphics/Vulkan/Surface.h>
#include <Core/Graphics/Vulkan/Swapchain.h>
#include <Core/Graphics/Vulkan/PipelineStash.h>
#include <Core/Graphics/Vulkan/RenderPassStash.h>
#include <Core/Graphics/Vulkan/CommandPool.h>
#include <Core/Graphics/Vulkan/Buffer.h>

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

   _renderPasses = std::make_unique<RenderPassStash>( *this );
   _pipelines    = std::make_unique<PipelineStash>( *this );
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
      _commandPools.push_back(
          std::make_unique<CommandPool>( *this, queueFamily.index, queueFamily.type ) );
   }
}

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
       [usage]( const std::unique_ptr<CommandPool>& pool ) { return usage & pool->getType(); } );
   if( it != _commandPools.end() )
   {
      // Found an adequate command pool
      return ( *it )->createCommandBuffer();
   }

   return nullptr;
}

std::shared_ptr<cyd::Buffer> cyd::Device::createStagingBuffer( size_t size, BufferUsageFlag usage )
{
   _buffers.push_back( std::make_shared<Buffer>(
       *this, size, usage, MemoryType::HOST_VISIBLE | MemoryType::HOST_COHERENT ) );
   return _buffers.back();
}

std::shared_ptr<cyd::Buffer> cyd::Device::createBuffer( size_t size, BufferUsageFlag usage )
{
   _buffers.push_back( std::make_shared<Buffer>( *this, size, usage, MemoryType::DEVICE_LOCAL ) );
   return _buffers.back();
}

// TODO Give possiblity of custom format and presentation mode using a create info as argument
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
   // Cleaning up device buffers
   for( auto& buffer : _buffers )
   {
      if( buffer.use_count() == 1 )
      {
         buffer.reset();
      }
   }
}

const VkQueue* cyd::Device::getQueue( uint32_t familyIndex, bool supportsPresentation ) const
{
   const std::vector<VkQueue>& vkQueues = _queueFamilies[familyIndex].queues;
   if( !vkQueues.empty() )
   {
      return &vkQueues[0];
   }

   return nullptr;
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

cyd::Device::~Device()
{
   vkDeviceWaitIdle( _vkDevice );

   for( auto& buffer : _buffers )
   {
      buffer.reset();
   }
   _pipelines.reset();
   _renderPasses.reset();
   _swapchain.reset();
   for( auto& commandPool : _commandPools )
   {
      commandPool.reset();
   }

   vkDestroyDevice( _vkDevice, nullptr );
}
