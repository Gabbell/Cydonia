#pragma once

#include "Common/Common.h"

#include <array>
#include <memory>
#include <optional>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkQueue );
FWDHANDLE( VkDevice );
FWDHANDLE( VkPhysicalDevice );
enum VkQueueFlagBits;
struct VkQueueFamilyProperties;
struct VkDeviceQueueCreateInfo;

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
   Device( const Instance* instance, const VkPhysicalDevice& physicalDevice );
   ~Device();

  private:
   // Queue internal struct
   struct Queue
   {
      uint32_t familyIndex;
      uint32_t index;
      uint32_t type;
      VkQueue vkQueue;
   };

   // Helper functions
   void _createLogicalDevice();

   std::optional<uint32_t> _pickQueueFamily(
       const VkQueueFlagBits desiredType,
       const std::vector<VkQueueFamilyProperties>& queueFamilies );

   void _addQueues(
       uint32_t index,
       const std::vector<VkQueueFamilyProperties>& queueFamilies,
       std::vector<VkDeviceQueueCreateInfo>& queueInfos );

   void _fetchQueues();

   // Member variables
   static constexpr uint32_t MAX_QUEUES = 32;

   // Queues
   // Since queues are destroyed when we destroy the device and we destroy the device only at
   // the end of life of the application, the pointers should not be invalidated.
   uint32_t _nbQueues = 0;  // Current number of queues
   std::array<Queue, MAX_QUEUES> _queues;
   std::vector<const Queue*> _graphicsQueues;
   std::vector<const Queue*> _computeQueues;
   std::vector<const Queue*> _transferQueues;
   std::vector<const Queue*> _presentQueues;

   // Used to keep track of which queue family indices have been used to create the device
   // Unfortunately, can't use a vector of bool and we don't know the number of families in advance
   // so we have to resort to this ugly thing
   std::vector<uint32_t> _queueFamiliesUsed;

   // Instance used to create the device
   const Instance* _attachedInstance = nullptr;

   // Logical device used for operations
   VkDevice _vkDevice = nullptr;

   // Contains physical limitations and properties of the device
   VkPhysicalDevice _physDevice = nullptr;
};
}  // namespace cyd
