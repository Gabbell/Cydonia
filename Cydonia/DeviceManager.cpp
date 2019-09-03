#include "DeviceManager.h"

#include "Assert.h"
#include "Instance.h"

#include <vulkan/vulkan.hpp>

cyd::DeviceManager::DeviceManager( const Instance* instance ) : _attachedInstance( instance )
{
   const vk::Instance& vkInstance       = _attachedInstance->getVKInstance();
   const vk::DispatchLoaderDynamic& dld = _attachedInstance->getDLD();

   auto result = vkInstance.enumeratePhysicalDevices( dld );
   CYDASSERT(
       result.result == vk::Result::eSuccess &&
       "DeviceManager: Could not enumerate physical devices" );

   std::vector<vk::PhysicalDevice> physDevices = result.value;
   CYDASSERT( !physDevices.empty() && "DeviceManager: No devices supporting Vulkan" );

   // TODO Add support for multiple devices
   for( const auto& physDevice : physDevices )
   {
      if( _checkDevice( physDevice ) )
      {
         // Found suitable device, add it to the currently managed devices
         _devices.emplace_back( _attachedInstance, physDevice );
         break;
      }
   }
}

bool cyd::DeviceManager::_checkDevice( const vk::PhysicalDevice& physDevice )
{
   const vk::PhysicalDeviceProperties& properties = physDevice.getProperties();
   const vk::PhysicalDeviceFeatures& features     = physDevice.getFeatures();
   const std::vector<vk::QueueFamilyProperties>& queueFamilies =
       physDevice.getQueueFamilyProperties();

   const bool isDiscrete = properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
   const bool supportsGraphics =
       std::find_if(
           queueFamilies.begin(),
           queueFamilies.end(),
           []( const vk::QueueFamilyProperties& properties ) {
              return ( properties.queueCount > 0 ) &&
                     ( properties.queueFlags & vk::QueueFlagBits::eGraphics );
           } ) != queueFamilies.end();
   const bool supportsTransfer =
       std::find_if(
           queueFamilies.begin(),
           queueFamilies.end(),
           []( const vk::QueueFamilyProperties& properties ) {
              return ( properties.queueCount > 0 ) &&
                     ( properties.queueFlags & vk::QueueFlagBits::eTransfer );
           } ) != queueFamilies.end();
   const bool supportsCompute =
       std::find_if(
           queueFamilies.begin(),
           queueFamilies.end(),
           []( const vk::QueueFamilyProperties& properties ) {
              return ( properties.queueCount > 0 ) &&
                     ( properties.queueFlags & vk::QueueFlagBits::eCompute );
           } ) != queueFamilies.end();

   const bool isChosen = isDiscrete && supportsGraphics && supportsTransfer && supportsCompute;
   if( isChosen )
   {
      // Device will be added to the device manager, dump some info
      fprintf(
          stdout,
          "DeviceManager: adding device to manager\n\tDevice Name: %s\n\tAPI Version: "
          "%u.%u.%u\n",
          properties.deviceName,
          VK_VERSION_MAJOR( properties.apiVersion ),
          VK_VERSION_MINOR( properties.apiVersion ),
          VK_VERSION_PATCH( properties.apiVersion ) );
   }

   return isChosen;
}

cyd::DeviceManager::~DeviceManager() {}