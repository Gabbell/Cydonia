#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <Graphics/Vulkan/Buffer.h>
#include <Graphics/Vulkan/Texture.h>

#include <Handles/HandleManager.h>

#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkQueue );
FWDHANDLE( VkDevice );
FWDHANDLE( VkPhysicalDevice );
struct VkPhysicalDeviceProperties;

namespace cyd
{
class Window;
struct TextureDescription;
}

namespace vk
{
class Instance;
class Surface;
class Swapchain;
class PipelineStash;
class RenderPassStash;
class SamplerStash;
class CommandPool;
class CommandBuffer;
class Texture;
class DescriptorPool;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class Device final
{
  public:
   Device(
       const cyd::Window& window,
       const Instance& instance,
       const Surface& surface,
       const VkPhysicalDevice& physDevice,
       const std::vector<const char*>& extensions );
   ~Device();

   // Interface
   // =============================================================================================
   Swapchain* createSwapchain( const cyd::SwapchainInfo& scInfo );
   CommandBuffer* createCommandBuffer( cyd::QueueUsageFlag usage, bool presentable = false );

   // Buffer creation function
   Buffer* createVertexBuffer( size_t size );
   Buffer* createIndexBuffer( size_t size );
   Buffer* createStagingBuffer( size_t size );
   Buffer* createUniformBuffer(
       size_t size,
       const cyd::ShaderObjectInfo& info,
       const cyd::DescriptorSetLayoutInfo& layout );
   Texture* createTexture(
       const cyd::TextureDescription& desc,
       const cyd::ShaderObjectInfo& info,
       const cyd::DescriptorSetLayoutInfo& layout );

   void cleanup();  // Clean up unused resources

   // Getters
   const VkPhysicalDevice& getPhysicalDevice() const noexcept { return _physDevice; }
   const VkDevice& getVKDevice() const noexcept { return _vkDevice; }
   const VkQueue* getQueueFromFamily( uint32_t familyIndex ) const;
   const VkQueue* getQueueFromUsage( cyd::QueueUsageFlag usage, bool supportsPresentation = false )
       const;

   Swapchain* getSwapchain() const { return _swapchain.get(); }

   PipelineStash& getPipelineStash() const { return *_pipelines; }
   RenderPassStash& getRenderPassStash() const { return *_renderPasses; }
   SamplerStash& getSamplerStash() const { return *_samplers; }

   // Support
   uint32_t findMemoryType( uint32_t typeFilter, uint32_t properties ) const;
   bool supportsPresentation() const;
   uint32_t maxPushConstantsSize() const;

  private:
   // =============================================================================================
   // Private Functions
   // =============================================================================================
   void _populateQueueFamilies();
   void _createLogicalDevice();
   void _fetchQueues();
   void _createCommandPools();
   void _createDescriptorPool();

   // Common buffer function
   Buffer* _createBuffer( size_t size, cyd::BufferUsageFlag usage, cyd::MemoryTypeFlag memoryType );
   void _destroyBuffer( Buffer* buffer );

   // =============================================================================================
   // Private Members
   // =============================================================================================
   const cyd::Window& _window;

   const Instance& _instance;
   const Surface& _surface;

   std::vector<Buffer> _buffers;
   std::vector<Texture> _textures;
   std::vector<std::unique_ptr<CommandPool>> _commandPools;

   std::unique_ptr<DescriptorPool> _descPool;
   std::unique_ptr<Swapchain> _swapchain;
   std::unique_ptr<RenderPassStash> _renderPasses;
   std::unique_ptr<SamplerStash> _samplers;
   std::unique_ptr<PipelineStash> _pipelines;

   struct QueueFamily
   {
      std::vector<VkQueue> queues;
      uint32_t index           = 0;
      uint32_t queueCount      = 0;
      cyd::QueueUsageFlag type = 0;
      bool supportsPresent     = false;
   };
   std::vector<QueueFamily> _queueFamilies;

   // Extensions used to create the device
   const std::vector<const char*>& _extensions;

   VkDevice _vkDevice           = nullptr;
   VkPhysicalDevice _physDevice = nullptr;
   std::unique_ptr<VkPhysicalDeviceProperties> _physProps;
};
}
