#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Types.h>

#include <memory>
#include <optional>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkQueue );
FWDHANDLE( VkDevice );
FWDHANDLE( VkPhysicalDevice );
struct VkDeviceQueueCreateInfo;

namespace cyd
{
class Instance;
class Window;
class Surface;
class Swapchain;
class PipelineStash;
class RenderPassStash;
class CommandPool;
class CommandBuffer;
struct SwapchainInfo;
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
       const Window& window,
       const Surface& surface,
       const VkPhysicalDevice& physDevice,
       const std::vector<const char*>& extensions );
   ~Device();

   // Getters
   const VkPhysicalDevice& getPhysicalDevice() const noexcept { return _physDevice; }
   const VkDevice& getVKDevice() const noexcept { return _vkDevice; }
   const VkQueue* getQueue( UsageFlag usage, bool supportsPresentation = false ) const;
   Swapchain* getSwapchain() const { return _swapchain.get(); }

   // These are used in the command buffers
   PipelineStash& getPipelineStash() const { return *_pipelines; }
   RenderPassStash& getRenderPassStash() const { return *_renderPasses; }

   // Interface
   std::shared_ptr<CommandBuffer> createCommandBuffer( UsageFlag usage );
   Swapchain* createSwapchain( const SwapchainInfo& scInfo );
   void cleanup(); // Clean up unused resources

   // Supports
   bool supportsPresentation() const;

  private:
   struct QueueFamily
   {
      bool used           = false;
      uint32_t index      = 0;
      uint32_t queueCount = 0;
      UsageFlag type      = Usage::UNKNOWN;
   };
   struct Queue
   {
      VkQueue vkQueue      = nullptr;
      bool supportsPresent = false;
      uint32_t familyIndex = 0;
      uint32_t index       = 0;
      UsageFlag type       = Usage::UNKNOWN;
   };

   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _populateQueueFamilies();
   void _createLogicalDevice();
   std::optional<QueueFamily> _pickQueueFamily( UsageFlag desiredType );
   void _addQueue(
       std::vector<VkDeviceQueueCreateInfo>& infos,
       float priority,
       uint32_t numQueues,
       const QueueFamily& family,
       const Surface& surface );
   void _fetchQueues();
   void _createCommandPools();

   // =============================================================================================
   // Private Members
   // =============================================================================================

   // Used to keep track of which queue family indices have been used to create the device. Used to
   // create the command pools.
   std::vector<QueueFamily> _queueFamilies;

   // Queues
   std::vector<Queue> _queues;

   // Extensions used to create the device
   const std::vector<const char*>& _extensions;

   std::unique_ptr<Swapchain> _swapchain;
   std::vector<std::unique_ptr<CommandPool>> _commandPools;
   std::unique_ptr<RenderPassStash> _renderPasses;
   std::unique_ptr<PipelineStash> _pipelines;

   const Instance& _instance;
   const Window& _window;
   const Surface& _surface;

   VkDevice _vkDevice           = nullptr;
   VkPhysicalDevice _physDevice = nullptr;
};
}
