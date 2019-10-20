#include <Core/Graphics/Device.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Instance.h>
#include <Core/Graphics/Surface.h>
#include <Core/Graphics/Swapchain.h>
#include <Core/Graphics/PipelineStash.h>
#include <Core/Graphics/RenderPassStash.h>
#include <Core/Graphics/CommandPool.h>

#include <algorithm>

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
      UsageFlag type = 0;

      VkQueueFlags vkQueueType = queueFamilies[i].queueFlags;
      if( vkQueueType & VK_QUEUE_GRAPHICS_BIT )
      {
         type |= static_cast<uint32_t>( Usage::GRAPHICS );
      }
      if( vkQueueType & VK_QUEUE_COMPUTE_BIT )
      {
         type |= static_cast<uint32_t>( Usage::COMPUTE );
      }
      if( vkQueueType & VK_QUEUE_TRANSFER_BIT )
      {
         type |= static_cast<uint32_t>( Usage::TRANSFER );
      }

      _queueFamilies.push_back( {false, i, queueFamilies[i].queueCount, type} );
   }
}

std::optional<cyd::Device::QueueFamily> cyd::Device::_pickQueueFamily( UsageFlag desiredType )
{
   auto it = std::find_if(
       _queueFamilies.begin(), _queueFamilies.end(), [desiredType]( const QueueFamily& family ) {
          return ( desiredType & family.type ) && !family.used;
       } );

   if( it != _queueFamilies.end() )
   {
      // Found an appropriate queue
      it->used = true;
      return *it;
   }

   // The desired type was not found in the queue families, return no value
   return std::nullopt;
}

void cyd::Device::_addQueue(
    std::vector<VkDeviceQueueCreateInfo>& infos,
    float priority,
    uint32_t numQueues,
    const QueueFamily& family,
    const Surface& surface )
{
   for( uint32_t i = 0; i < numQueues; ++i )
   {
      // Checking for presentation support
      VkBool32 supportsPresent = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(
          _physDevice, family.index, surface.getVKSurface(), &supportsPresent );

      Queue queue           = {};
      queue.familyIndex     = family.index;
      queue.index           = i;
      queue.supportsPresent = static_cast<bool>( supportsPresent );
      queue.type            = family.type;

      _queues.push_back( std::move( queue ) );
   }

   // Add queue(s) to the queue infos to later be create with the logical
   VkDeviceQueueCreateInfo queueInfo = {};
   queueInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
   queueInfo.pNext                   = nullptr;
   queueInfo.flags                   = 0;
   queueInfo.queueFamilyIndex        = family.index;
   queueInfo.queueCount              = numQueues;
   queueInfo.pQueuePriorities        = &priority;

   infos.push_back( std::move( queueInfo ) );
}

void cyd::Device::_createLogicalDevice()
{
   // Populating queue infos
   std::vector<VkDeviceQueueCreateInfo> queueInfos;

   // Picking and potentially adding graphics queues
   const std::optional<QueueFamily> graphicsFam = _pickQueueFamily( Usage::GRAPHICS );
   if( graphicsFam.has_value() )
   {
      _addQueue( queueInfos, 1.0f, 2, graphicsFam.value(), _surface );
   }

   // Separate transfer queue because why not
   const std::optional<QueueFamily> transferFam = _pickQueueFamily( Usage::TRANSFER );
   if( transferFam.has_value() )
   {
      _addQueue( queueInfos, 1.0f, 2, transferFam.value(), _surface );
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
   for( Queue& queue : _queues )
   {
      vkGetDeviceQueue( _vkDevice, queue.familyIndex, queue.index, &queue.vkQueue );
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

std::shared_ptr<cyd::CommandBuffer> cyd::Device::createCommandBuffer( UsageFlag usage )
{
   const auto it = std::find_if(
       _commandPools.begin(),
       _commandPools.end(),
       [usage]( const std::unique_ptr<CommandPool>& pool ) { return usage & pool->getType(); } );

   if( it != _commandPools.end() )
   {
      // Found a proper command pool
      return ( *it )->createCommandBuffer();
   }

   return nullptr;
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
   for( auto& commandPool : _commandPools )
   {
      commandPool->cleanup();
   }
}

const VkQueue* cyd::Device::getQueue( cyd::UsageFlag usage, bool supportsPresentation ) const
{
   // Check to see if one of the queues has at least the usage that we want and it MUST support
   // presentation if we want it
   const auto it = std::find_if(
       _queues.begin(), _queues.end(), [usage, supportsPresentation]( const Queue& queue ) {
          return ( usage & queue.type ) && !( supportsPresentation && !queue.supportsPresent );
       } );
   if( it != _queues.end() )
   {
      return &it->vkQueue;
   }
   return nullptr;
}

bool cyd::Device::supportsPresentation() const
{
   const auto it = std::find_if(
       _queues.begin(), _queues.end(), []( const Queue& queue ) { return queue.supportsPresent; } );
   if( it != _queues.end() )
   {
      return true;
   }
   return false;
}

cyd::Device::~Device()
{
   vkDeviceWaitIdle( _vkDevice );

   _pipelines.reset();
   _renderPasses.reset();
   _swapchain.reset();
   for( auto& commandPool : _commandPools )
   {
      commandPool.reset();
   }

   vkDestroyDevice( _vkDevice, nullptr );
}
