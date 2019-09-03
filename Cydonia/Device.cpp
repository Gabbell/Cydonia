#include "Device.h"

#include "Assert.h"
#include "Instance.h"

cyd::Device::Device( const cyd::Instance* instance, const vk::PhysicalDevice& physDevice )
    : _attachedInstance( instance ), _physDevice( std::move( physDevice ) )
{
   _createLogicalDevice();
   _createQueues();
}

std::optional<uint32_t> cyd::Device::_pickQueue( vk::QueueFlagBits desiredType )
{
   for( uint32_t i = 0; i < _queueFamilies.size(); ++i )
   {
      // If we find a queue family that match the type we want and it is not being used, use
      if( ( desiredType & _queueFamilies[i].queueFlags ) == desiredType && !_queueFamilyUsed[i] )
      {
         _queueFamilyUsed[i] = true;
         return i;
      }
   }

   // The desired type was not found in the queue families, return no value
   return std::nullopt;
}

void cyd::Device::_createLogicalDevice()
{
   // Populating queue families
   _queueFamilies = _physDevice.getQueueFamilyProperties();
   _queueFamilyUsed.resize( _queueFamilies.size() );

   // TODO More dynamic queue creation
   const float priorities = 1.0f;
   std::vector<vk::DeviceQueueCreateInfo> queueInfos;

   // Graphics queue
   const std::optional<uint32_t> graphicsIdx = _pickQueue( vk::QueueFlagBits::eGraphics );
   if( graphicsIdx.has_value() )
   {
      queueInfos.emplace_back( vk::DeviceQueueCreateFlags(), graphicsIdx.value(), 1, &priorities );
   }

   // Separate transfer queue because why not
   const std::optional<uint32_t> transferIdx = _pickQueue( vk::QueueFlagBits::eTransfer );
   if( transferIdx.has_value() )
   {
      queueInfos.emplace_back( vk::DeviceQueueCreateFlags(), transferIdx.value(), 1, &priorities );
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
           .setPEnabledFeatures( &_physDevice.getFeatures() );

   auto result = _physDevice.createDevice( deviceInfo, nullptr, _attachedInstance->getDLD() );
   CYDASSERT(
       result.result == vk::Result::eSuccess && "VEDevice: Could not create logical device" );
   _vkDevice = result.value;
}

void cyd::Device::_createQueues() {}

cyd::Device::~Device() { _vkDevice.destroy(); }
