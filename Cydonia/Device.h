#pragma once

#include <optional>
#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
namespace vk
{
class PhysicalDevice;
class Device;
class Queue;
struct QueueFamilyProperties;
struct DeviceQueueCreateInfo;
enum class QueueFlagBits;
}

namespace cyd
{
class Instance;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Device
{
  public:
   Device( const Instance* instance, const vk::PhysicalDevice&& physicalDevice );
   ~Device();

  private:
   // Queue internal struct
   struct Queue
   {
      uint32_t familyIndex;
      uint32_t index;
      uint32_t type;
      std::unique_ptr<vk::Queue> vkQueue;
   };

   // Helper functions
   void _createLogicalDevice();

   std::optional<uint32_t> _pickQueueFamily(
       const vk::QueueFlagBits desiredType,
       const std::vector<vk::QueueFamilyProperties>& queueFamilies );

   void _addQueues(
       uint32_t index,
       const std::vector<vk::QueueFamilyProperties>& queueFamilies,
       std::vector<vk::DeviceQueueCreateInfo>& queueInfos );

   void _fetchQueues();

   // Queues
   // Since queues are destroyed when we destroy the device and we destroy the device only at
   // the end of life of the application, the pointers should not be invalidated.
   std::vector<Queue> _queues;
   std::vector<const Queue*> _graphicsQueues;
   std::vector<const Queue*> _computeQueues;
   std::vector<const Queue*> _transferQueues;

   // Used to keep track of which queue family indices have been used to create the device
   std::vector<bool> _queueFamilyUsed;

   // Instance used to create the device
   const Instance* _attachedInstance = nullptr;

   // Logical device used for operations
   std::unique_ptr<vk::Device> _vkDevice;

   // Contains physical limitations and properties of the device
   std::unique_ptr<vk::PhysicalDevice> _physDevice;
};
}
