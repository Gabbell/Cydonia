#pragma once

#include <vulkan/vulkan.hpp>

#include <optional>

namespace cyd
{
// Forwards
class Instance;

// Definition
class Device
{
  public:
   Device( const Instance* instance, const vk::PhysicalDevice& physicalDevice );
   ~Device();

  private:
   std::optional<uint32_t> _pickQueue( vk::QueueFlagBits desiredType );

   void _createLogicalDevice();
   void _createQueues();

   // Used to keep track of which queue family indices have been used to create the device
   std::vector<bool> _queueFamilyUsed;
   std::vector<vk::QueueFamilyProperties> _queueFamilies;

   // Instance used to create the device
   const Instance* _attachedInstance = nullptr;

   // Contains physical limitations and properties of the device
   vk::PhysicalDevice _physDevice;

   // Logical device used for operations
   vk::Device _vkDevice;

   // Queues
   vk::Queue _graphicsQueue;
   vk::Queue _transferQueue;
};
}  // namespace cyd
