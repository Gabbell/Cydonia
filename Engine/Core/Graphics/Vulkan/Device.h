#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

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
struct VkPhysicalDeviceProperties;

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
class Buffer;
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

   // Interface
   Swapchain* createSwapchain( const SwapchainInfo& scInfo );
   std::shared_ptr<CommandBuffer> createCommandBuffer(
       QueueUsageFlag usage,
       bool presentable = false );
   std::shared_ptr<Buffer> createStagingBuffer( size_t size, BufferUsageFlag usage );
   std::shared_ptr<Buffer> createBuffer( size_t size, BufferUsageFlag usage );
   void cleanup();  // Clean up unused resources

   // Getters
   const VkPhysicalDevice& getPhysicalDevice() const noexcept { return _physDevice; }
   const VkDevice& getVKDevice() const noexcept { return _vkDevice; }
   const VkQueue* getQueueFromFamily( uint32_t familyIndex ) const;
   const VkQueue* getQueueFromUsage( QueueUsageFlag usage, bool supportsPresentation = false )
       const;
   Swapchain* getSwapchain() const { return _swapchain.get(); }

   // These are used in the command buffers
   PipelineStash& getPipelineStash() const { return *_pipelines; }
   RenderPassStash& getRenderPassStash() const { return *_renderPasses; }

   // Supports
   bool supportsPresentation() const;
   uint32_t maxPushConstantsSize() const;

  private:
   struct QueueFamily
   {
      std::vector<VkQueue> queues;
      uint32_t index       = 0;
      uint32_t queueCount  = 0;
      QueueUsageFlag type  = 0;
      bool supportsPresent = false;
   };

   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _populateQueueFamilies();
   void _createLogicalDevice();
   void _fetchQueues();
   void _createCommandPools();

   // =============================================================================================
   // Private Members
   // =============================================================================================
   std::vector<QueueFamily> _queueFamilies;

   // Extensions used to create the device
   const std::vector<const char*>& _extensions;

   std::vector<std::shared_ptr<Buffer>> _buffers;
   std::vector<std::unique_ptr<CommandPool>> _commandPools;
   std::unique_ptr<Swapchain> _swapchain;
   std::unique_ptr<RenderPassStash> _renderPasses;
   std::unique_ptr<PipelineStash> _pipelines;

   const Instance& _instance;
   const Window& _window;
   const Surface& _surface;

   VkDevice _vkDevice           = nullptr;
   VkPhysicalDevice _physDevice = nullptr;
   std::unique_ptr<VkPhysicalDeviceProperties> _physProps;
};
}
