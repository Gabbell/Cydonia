#pragma once

#include <Core/Common/Common.h>

#include <array>
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
class Surface;
class Swapchain;
struct Extent;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Device
{
  public:
   Device(
       const Instance& instance,
       const Surface& surface,
       const VkPhysicalDevice& physDevice,
       const std::vector<const char*>& extensions );
   ~Device();

   // Interface
   const Swapchain* createSwapchain( const Extent& extent );

   // Getters
   const VkPhysicalDevice& getPhysicalDevice() const noexcept { return _physDevice; }
   const VkDevice& getVKDevice() const noexcept { return _vkDevice; }

   // Supports
   bool supportsPresentation() const noexcept { return !_presentQueues.empty(); }

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _createLogicalDevice();

   VkDeviceQueueCreateInfo _createQueues(
       uint32_t famIndex,
       const VkQueueFamilyProperties& queueFamily,
       const Surface& surface );

   std::optional<uint32_t> _pickQueueFamily(
       const VkQueueFlagBits desiredType,
       const std::vector<VkQueueFamilyProperties>& queueFamilies );

   void _fetchQueues();

   // Queues
   static constexpr uint32_t MAX_QUEUES = 32;
   uint32_t _nbQueues                   = 0;
   // Used to keep track of which queue family indices have been used to create the device
   std::vector<uint32_t> _queueFamiliesUsed;
   struct Queue
   {
      uint32_t familyIndex;
      uint32_t index;
      uint32_t type;
	  bool supportsPresent;
      VkQueue vkQueue;
   };

   std::vector<const Queue*> _graphicsQueues;
   std::vector<const Queue*> _computeQueues;
   std::vector<const Queue*> _transferQueues;
   std::vector<const Queue*> _presentQueues;
   std::array<Queue, MAX_QUEUES> _queues;  // Where the queues are actually stored

   // Swapchain
   std::unique_ptr<Swapchain> _swapchain;

   // Extensions used to create the device
   const std::vector<const char*>& _extensions;

   const Instance& _instance;
   const Surface& _surface;

   VkDevice _vkDevice           = nullptr;
   VkPhysicalDevice _physDevice = nullptr;
};
}
