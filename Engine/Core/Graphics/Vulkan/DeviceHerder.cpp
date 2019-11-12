#include <Core/Graphics/Vulkan/DeviceHerder.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Window/Window.h>

#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/Instance.h>
#include <Core/Graphics/Vulkan/Surface.h>

#include <set>

cyd::DeviceHerder::DeviceHerder(
    const Instance& instance,
    const Window& window,
    const Surface& surface )
    : _instance( instance ), _window( window ), _surface( surface )
{
   // Desired extensions to be used when creating logical devices
   _extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

   uint32_t physicalDeviceCount;
   vkEnumeratePhysicalDevices( instance.getVKInstance(), &physicalDeviceCount, nullptr );

   std::vector<VkPhysicalDevice> physicalDevices( physicalDeviceCount );
   VkResult result = vkEnumeratePhysicalDevices(
       instance.getVKInstance(), &physicalDeviceCount, physicalDevices.data() );

   CYDASSERT( result == VK_SUCCESS && "DeviceHerder: Could not enumerate physical devices" );

   CYDASSERT( !physicalDevices.empty() && "DeviceHerder: No devices supporting Vulkan" );

   // TODO Add support for multiple devices
   for( const auto& physDevice : physicalDevices )
   {
      if( _checkDevice( _surface, physDevice ) )
      {
         // Found suitable device, add it to the currently managed devices
         _devices.emplace_back(
             std::make_unique<Device>( _instance, _window, _surface, physDevice, _extensions ) );
         break;
      }
   }
}

bool cyd::DeviceHerder::_checkDevice( const Surface& surface, const VkPhysicalDevice& physDevice )
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

   VkBool32 supportsGraphics = false;
   VkBool32 supportsTransfer = false;
   VkBool32 supportsCompute  = false;
   VkBool32 supportsPresent  = false;

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

         if( !supportsPresent )
         {
            vkGetPhysicalDeviceSurfaceSupportKHR(
                physDevice, i, surface.getVKSurface(), &supportsPresent );
         }
      }
   }

   const bool supportsExtensions = _checkDeviceExtensionSupport( physDevice );

   const bool isChosen = isDiscrete && supportsGraphics && supportsTransfer && supportsCompute &&
                         supportsPresent && supportsExtensions;
   if( isChosen )
   {
      // Device will be added to the device manager, dump some info
      fprintf(
          stdout,
          "DeviceHerder: adding device to manager\n\tDevice Name: %s\n\tAPI Version: "
          "%u.%u.%u\n",
          properties.deviceName,
          VK_VERSION_MAJOR( properties.apiVersion ),
          VK_VERSION_MINOR( properties.apiVersion ),
          VK_VERSION_PATCH( properties.apiVersion ) );
   }

   return isChosen;
}

bool cyd::DeviceHerder::_checkDeviceExtensionSupport( const VkPhysicalDevice& physDevice )
{
   uint32_t extensionCount;
   vkEnumerateDeviceExtensionProperties( physDevice, nullptr, &extensionCount, nullptr );

   std::vector<VkExtensionProperties> supportedExtensions( extensionCount );
   vkEnumerateDeviceExtensionProperties(
       physDevice, nullptr, &extensionCount, supportedExtensions.data() );

   std::set<std::string> requiredExtensions( _extensions.begin(), _extensions.end() );
   for( const auto& extension : supportedExtensions )
   {
      requiredExtensions.erase( extension.extensionName );
   }

   return requiredExtensions.empty();
}

const cyd::Swapchain* cyd::DeviceHerder::getMainSwapchain() { return _devices[0]->getSwapchain(); }

cyd::DeviceHerder::~DeviceHerder() {}
