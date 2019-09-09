#include "Device.h"

#include "Common/Assert.h"
#include "Core/Instance.h"

#include <vulkan/vulkan.h>

cyd::Device::Device( const cyd::Instance* instance, const VkPhysicalDevice& physDevice )
    : _attachedInstance( instance ), _physDevice( std::move( physDevice ) )
{
   _createLogicalDevice();
   _fetchQueues();
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
      _addQueues( graphicsIdx.value(), queueFamilies, queueInfos );
   }

   // Separate transfer queue because why not
   const std::optional<uint32_t> transferIdx =
       _pickQueueFamily( VK_QUEUE_TRANSFER_BIT, queueFamilies );
   if( transferIdx.has_value() )
   {
      _addQueues( transferIdx.value(), queueFamilies, queueInfos );
   }

   // Fetch layers
   std::vector<const char*> layers;
   if( _attachedInstance )
   {
      layers = _attachedInstance->getLayers();
   }
   else
   {
      CYDASSERT( !"Device: No instance to fetch layers from" );
   }

   // Create logical device
   VkPhysicalDeviceFeatures deviceFeatures = {};
   vkGetPhysicalDeviceFeatures( _physDevice, &deviceFeatures );

   VkDeviceCreateInfo deviceInfo      = {};
   deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   deviceInfo.pNext                   = nullptr;
   deviceInfo.flags                   = 0;
   deviceInfo.queueCreateInfoCount    = static_cast<uint32_t>( queueInfos.size() );
   deviceInfo.pQueueCreateInfos       = queueInfos.data();
   deviceInfo.enabledLayerCount       = static_cast<uint32_t>( layers.size() );
   deviceInfo.ppEnabledLayerNames     = layers.data();
   deviceInfo.enabledExtensionCount   = 0;
   deviceInfo.ppEnabledExtensionNames = nullptr;
   deviceInfo.pEnabledFeatures        = &deviceFeatures;

   VkResult result = vkCreateDevice( _physDevice, &deviceInfo, nullptr, &_vkDevice );
   CYDASSERT( result == VK_SUCCESS && "VEDevice: Could not create logical device" );
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

void cyd::Device::_addQueues(
    uint32_t famIndex,
    const std::vector<VkQueueFamilyProperties>& queueFamilies,
    std::vector<VkDeviceQueueCreateInfo>& queueInfos )
{
   // TODO More dynamic queue creation
   const float priorities = 1.0f;

   // TODO Just use 1 queue for now
   const uint32_t numQueues  = 1;
   const uint32_t actualType = static_cast<uint32_t>( queueFamilies[famIndex].queueFlags );

   for( uint32_t i = 0; i < numQueues; ++i )
   {
      Queue queue;
      queue.familyIndex = famIndex;
      queue.index       = i;
      queue.type        = actualType;

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

      // Adding present queue
      if( _attachedInstance )
      {
         VkBool32 supportsPresent = false;
         vkGetPhysicalDeviceSurfaceSupportKHR(
             _physDevice, famIndex, _attachedInstance->getSurface(), &supportsPresent );
         if( supportsPresent )
         {
            _presentQueues.push_back( &_queues[_nbQueues] );
         }
      }
      else
      {
         CYDASSERT( !"Device: Not instance to determine if queue supports presentation" );
      }

      _nbQueues++;
   }

   // Add queue(s) to the queue infos to later be create with the logical
   VkDeviceQueueCreateInfo queueInfo = {};
   queueInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
   queueInfo.pNext                   = nullptr;
   queueInfo.flags                   = 0;
   queueInfo.queueFamilyIndex        = famIndex;
   queueInfo.queueCount              = numQueues;
   queueInfo.pQueuePriorities        = &priorities;

   queueInfos.push_back( std::move( queueInfo ) );
}

void cyd::Device::_fetchQueues()
{
   for( uint32_t i = 0; i < _nbQueues; ++i )
   {
      vkGetDeviceQueue( _vkDevice, _queues[i].familyIndex, _queues[i].index, &_queues[i].vkQueue );
   }
}

cyd::Device::~Device() { vkDestroyDevice( _vkDevice, nullptr ); }
