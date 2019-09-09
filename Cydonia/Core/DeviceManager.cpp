#include "DeviceManager.h"

#include "Common/Assert.h"
#include "Core/Instance.h"
#include "Core/Device.h"

#include <vulkan/vulkan.h>

cyd::DeviceManager::DeviceManager( const Instance* instance ) : _attachedInstance( instance )
{
   CYDASSERT( _attachedInstance && "DeviceManager: Can't create device manager without instance" );
   if( _attachedInstance )
   {
      const VkInstance vkInstance = _attachedInstance->getVKInstance();

      uint32_t physicalDeviceCount;
      vkEnumeratePhysicalDevices( vkInstance, &physicalDeviceCount, nullptr );

      std::vector<VkPhysicalDevice> physicalDevices( physicalDeviceCount );
      VkResult result =
          vkEnumeratePhysicalDevices( vkInstance, &physicalDeviceCount, physicalDevices.data() );

      CYDASSERT( result == VK_SUCCESS && "DeviceManager: Could not enumerate physical devices" );

      CYDASSERT( !physicalDevices.empty() && "DeviceManager: No devices supporting Vulkan" );

      // TODO Add support for multiple devices
      for( const auto& physDevice : physicalDevices )
      {
         if( _checkDevice( physDevice ) )
         {
            // Found suitable device, add it to the currently managed devices
            _devices.emplace_back(
                std::make_unique<Device>( _attachedInstance, std::move( physDevice ) ) );
            break;
         }
      }
   }
}

bool cyd::DeviceManager::_checkDevice( const VkPhysicalDevice& physDevice )
{
   VkPhysicalDeviceProperties properties = {};
   vkGetPhysicalDeviceProperties( physDevice, &properties );

   VkPhysicalDeviceFeatures features = {};
   vkGetPhysicalDeviceFeatures( physDevice, &features );

   uint32_t queueFamilyCount = 0;
   vkGetPhysicalDeviceQueueFamilyProperties( physDevice, &queueFamilyCount, nullptr );

   std::vector<VkQueueFamilyProperties> queueFamilies( queueFamilyCount );
   vkGetPhysicalDeviceQueueFamilyProperties( physDevice, &queueFamilyCount, queueFamilies.data() );

   const bool isDiscrete = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;

   VkBool32 supportsGraphics, supportsTransfer, supportsCompute, supportsPresent = false;

   for( uint32_t i = 0; i < queueFamilies.size(); ++i )
   {
      if( queueFamilies[i].queueCount > 0 )
      {
         if( queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT )
         {
            supportsGraphics = true;
         }
         if( queueFamilies[i].queueFlags & VK_QUEUE_TRANSFER_BIT )
         {
            supportsTransfer = true;
         }
         if( queueFamilies[i].queueFlags & VK_QUEUE_COMPUTE_BIT )
         {
            supportsCompute = true;
         }

         if( _attachedInstance && !supportsPresent )
         {
            vkGetPhysicalDeviceSurfaceSupportKHR(
                physDevice, i, _attachedInstance->getSurface(), &supportsPresent );
         }
      }
   }

   const bool isChosen = isDiscrete && supportsGraphics && supportsTransfer && supportsCompute && supportsPresent;
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
