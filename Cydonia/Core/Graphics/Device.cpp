#include <Core/Graphics/Device.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Instance.h>
#include <Core/Graphics/Surface.h>
#include <Core/Graphics/Swapchain.h>

cyd::Device::Device(
    const Instance& instance,
    const Surface& surface,
    const VkPhysicalDevice& physDevice,
    const std::vector<const char*>& extensions )
    : _instance( instance ),
      _surface( surface ),
      _extensions( extensions ),
      _physDevice( physDevice )
{
   _createLogicalDevice();
   _fetchQueues();
}

// TODO Give possiblity of custom format and presentation mode using a create info as argument
const cyd::Swapchain* cyd::Device::createSwapchain( const Extent& extent )
{
   if( !_presentQueues.empty() )
   {
      _swapchain = std::make_unique<Swapchain>( *this, _surface, extent );
      return _swapchain.get();
   }

   CYDASSERT(
       !"Device: No present queue to create swapchain. Does the device support presentation?" );
   return nullptr;
}

void cyd::Device::_createLogicalDevice()
{
   // Populating queue families
   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties( _physDevice, &queueFamilyCount, nullptr );

   std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
   vkGetPhysicalDeviceQueueFamilyProperties( _physDevice, &queueFamilyCount, queueFamilies.data() );

   // Populating queue infos
   std::vector<VkDeviceQueueCreateInfo> queueInfos;

   // Picking and potentially adding graphics queues
   const std::optional<uint32_t> graphicsIdx =
       _pickQueueFamily( VK_QUEUE_GRAPHICS_BIT, queueFamilies );
   if( graphicsIdx.has_value() )
   {
      const uint32_t famIndex = graphicsIdx.value();
      queueInfos.push_back(
          std::move( _createQueues( famIndex, queueFamilies[famIndex], _surface ) ) );
   }

   // Separate transfer queue because why not
   const std::optional<uint32_t> transferIdx =
       _pickQueueFamily( VK_QUEUE_TRANSFER_BIT, queueFamilies );
   if( transferIdx.has_value() )
   {
      const uint32_t famIndex = transferIdx.value();
      queueInfos.push_back(
          std::move( _createQueues( famIndex, queueFamilies[famIndex], _surface ) ) );
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

VkDeviceQueueCreateInfo cyd::Device::_createQueues(
    uint32_t famIndex,
    const VkQueueFamilyProperties& queueFamily,
    const Surface& surface )
{
   // TODO More dynamic queue creation
   const float priorities = 1.0f;

   // TODO Just use 1 queue for now
   const uint32_t numQueues  = 1;
   const uint32_t actualType = static_cast<uint32_t>( queueFamily.queueFlags );

   for( uint32_t i = 0; i < numQueues; ++i )
   {
      // Checking for presentation support
      VkBool32 supportsPresent = false;
      vkGetPhysicalDeviceSurfaceSupportKHR(
          _physDevice, famIndex, surface.getVKSurface(), &supportsPresent );

      Queue queue           = {};
      queue.familyIndex     = famIndex;
      queue.index           = i;
      queue.supportsPresent = static_cast<bool>( supportsPresent );
      queue.type            = actualType;

      _queues[_nbQueues] = std::move( queue );

      // Adding pointers to the previously added queue into the specific queue type vectors
      if( actualType & VK_QUEUE_GRAPHICS_BIT )
      {
         _graphicsQueues.push_back( &_queues[_nbQueues] );
      }
      if( actualType & VK_QUEUE_COMPUTE_BIT )
      {
         _computeQueues.push_back( &_queues[_nbQueues] );
      }
      if( actualType & VK_QUEUE_TRANSFER_BIT )
      {
         _transferQueues.push_back( &_queues[_nbQueues] );
      }
      if( supportsPresent )
      {
         _presentQueues.push_back( &_queues[_nbQueues] );
      }

      ++_nbQueues;
   }

   // Add queue(s) to the queue infos to later be create with the logical
   VkDeviceQueueCreateInfo queueInfo = {};
   queueInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
   queueInfo.pNext                   = nullptr;
   queueInfo.flags                   = 0;
   queueInfo.queueFamilyIndex        = famIndex;
   queueInfo.queueCount              = numQueues;
   queueInfo.pQueuePriorities        = &priorities;

   return queueInfo;
}

std::optional<uint32_t> cyd::Device::_pickQueueFamily(
    const VkQueueFlagBits desiredType,
    const std::vector<VkQueueFamilyProperties>& queueFamilies )
{
   for( uint32_t i = 0; i < queueFamilies.size(); ++i )
   {
      // Determine if we have previously used this family. They need to be unique per VkDevice.
      const bool queueFamilyUsed =
          std::find( _queueFamiliesUsed.begin(), _queueFamiliesUsed.end(), i ) !=
          _queueFamiliesUsed.end();

      // If we find a queue family that match the type we want and it is not being used, use
      if( ( desiredType & queueFamilies[i].queueFlags ) == desiredType && !queueFamilyUsed )
      {
         _queueFamiliesUsed.push_back( i );
         return i;
      }
   }

   // The desired type was not found in the queue families, return no value
   return std::nullopt;
}

void cyd::Device::_fetchQueues()
{
   for( uint32_t i = 0; i < _nbQueues; ++i )
   {
      vkGetDeviceQueue( _vkDevice, _queues[i].familyIndex, _queues[i].index, &_queues[i].vkQueue );
   }
}

cyd::Device::~Device()
{
   vkDestroySwapchainKHR( _vkDevice, _swapchain->getVKSwapchain(), nullptr );
   vkDestroyDevice( _vkDevice, nullptr );
}
