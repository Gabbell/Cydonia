#include "Device.h"

#include "Assert.h"
#include "Instance.h"

#include <vulkan/vulkan.hpp>

cyd::Device::Device( const cyd::Instance* instance, const vk::PhysicalDevice&& physDevice )
    : _attachedInstance( instance ),
      _physDevice( std::make_unique<vk::PhysicalDevice>( physDevice ) )
{
   _createLogicalDevice();
   _fetchQueues();
}

void cyd::Device::_createLogicalDevice()
{
   // Populating queue families
   std::vector<vk::QueueFamilyProperties> queueFamilies = _physDevice->getQueueFamilyProperties();
   _queueFamilyUsed.resize( queueFamilies.size() );

   std::vector<vk::DeviceQueueCreateInfo> queueInfos;

   // Picking and potentially adding qraphics queues
   const std::optional<uint32_t> graphicsIdx =
       _pickQueueFamily( vk::QueueFlagBits::eGraphics, queueFamilies );
   if( graphicsIdx.has_value() )
   {
      _addQueues( graphicsIdx.value(), queueFamilies, queueInfos );
   }

   // Separate transfer queue because why not
   const std::optional<uint32_t> transferIdx =
       _pickQueueFamily( vk::QueueFlagBits::eTransfer, queueFamilies );
   if( transferIdx.has_value() )
   {
      _addQueues( transferIdx.value(), queueFamilies, queueInfos );
   }

   // Create logical device
   const std::vector<const char*> layers = _attachedInstance->getLayers();

   vk::DeviceCreateInfo deviceInfo =
       vk::DeviceCreateInfo()
           .setFlags( vk::DeviceCreateFlags() )
           .setQueueCreateInfoCount( static_cast<uint32_t>( queueInfos.size() ) )
           .setPQueueCreateInfos( queueInfos.data() )
           .setEnabledLayerCount( static_cast<uint32_t>( layers.size() ) )
           .setPpEnabledLayerNames( layers.data() )
           .setEnabledExtensionCount( 0 )
           .setPpEnabledExtensionNames( nullptr )
           .setPEnabledFeatures( &_physDevice->getFeatures() );

   auto result = _physDevice->createDevice( deviceInfo, nullptr );
   CYDASSERT(
       result.result == vk::Result::eSuccess && "VEDevice: Could not create logical device" );
   _vkDevice = std::make_unique<vk::Device>( std::move( result.value ) );
}

std::optional<uint32_t> cyd::Device::_pickQueueFamily(
    const vk::QueueFlagBits desiredType,
    const std::vector<vk::QueueFamilyProperties>& queueFamilies )
{
   for( uint32_t i = 0; i < queueFamilies.size(); ++i )
   {
      // If we find a queue family that match the type we want and it is not being used, use
      if( ( desiredType & queueFamilies[i].queueFlags ) == desiredType && !_queueFamilyUsed[i] )
      {
         _queueFamilyUsed[i] = true;
         return i;
      }
   }

   // The desired type was not found in the queue families, return no value
   return std::nullopt;
}

void cyd::Device::_addQueues(
    uint32_t famIndex,
    const std::vector<vk::QueueFamilyProperties>& queueFamilies,
    std::vector<vk::DeviceQueueCreateInfo>& queueInfos )
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

      _queues.push_back( std::move( queue ) );

      // Adding pointers to the previously added queue into the specific queue type vectors
      if( actualType & static_cast<uint32_t>( vk::QueueFlagBits::eGraphics ) )
      {
         _graphicsQueues.push_back( &_queues.back() );
      }
      if( actualType & static_cast<uint32_t>( vk::QueueFlagBits::eCompute ) )
      {
         _computeQueues.push_back( &_queues.back() );
      }
      if( actualType & static_cast<uint32_t>( vk::QueueFlagBits::eTransfer ) )
      {
         _transferQueues.push_back( &_queues.back() );
      }
   }

   // Add queue(s) to the queue infos to later be create with the logical device
   queueInfos.emplace_back( vk::DeviceQueueCreateFlags(), famIndex, numQueues, &priorities );
}

void cyd::Device::_fetchQueues()
{
   for( auto& queue : _queues )
   {
      queue.vkQueue =
          std::make_unique<vk::Queue>( _vkDevice->getQueue( queue.familyIndex, queue.index ) );
   }
}

cyd::Device::~Device() { _vkDevice->destroy(); }
