#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <memory>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkQueue );
FWDHANDLE( VkDevice );
FWDHANDLE( VkPhysicalDevice );
struct VkPhysicalDeviceProperties;

namespace CYD
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
class Buffer;
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
       const CYD::Window& window,
       const Instance& instance,
       const Surface& surface,
       const VkPhysicalDevice& physDevice,
       const std::vector<const char*>& extensions );
   ~Device();

   // Interface
   // =============================================================================================
   Swapchain* createSwapchain( const CYD::SwapchainInfo& scInfo );
   CommandBuffer* createCommandBuffer( CYD::QueueUsageFlag usage, bool presentable = false );

   // Buffer creation function
   Buffer* createVertexBuffer( size_t size );
   Buffer* createIndexBuffer( size_t size );
   Buffer* createStagingBuffer( size_t size );
   Buffer* createUniformBuffer( size_t size );
   Buffer* createBuffer( size_t size );
   Texture* createTexture( const CYD::TextureDescription& desc );

   void cleanup();  // Clean up unused resources

   // Getters
   const VkPhysicalDevice& getPhysicalDevice() const noexcept { return m_physDevice; }
   const VkDevice& getVKDevice() const noexcept { return m_vkDevice; }
   const VkQueue* getQueueFromFamily( uint32_t familyIndex ) const;
   const VkQueue* getQueueFromUsage( CYD::QueueUsageFlag usage, bool supportsPresentation = false )
       const;

   Swapchain* getSwapchain() const { return m_swapchain.get(); }

   PipelineStash& getPipelineStash() const { return *m_pipelines; }
   RenderPassStash& getRenderPassStash() const { return *m_renderPasses; }
   SamplerStash& getSamplerStash() const { return *m_samplers; }
   DescriptorPool& getDescriptorPool() const { return *m_descPool; }

   // Support
   uint32_t findMemoryType( uint32_t typeFilter, uint32_t properties ) const;
   bool supportsPresentation() const;
   const VkPhysicalDeviceProperties* getProperties() const { return m_physProps.get(); }

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
   Buffer* _createBuffer( size_t size, CYD::BufferUsageFlag usage, CYD::MemoryTypeFlag memoryType );
   void _destroyBuffer( Buffer* buffer );

   // =============================================================================================
   // Private Members
   // =============================================================================================
   const CYD::Window& m_window;

   const Instance& m_instance;
   const Surface& m_surface;

   std::vector<Buffer> m_buffers;
   std::vector<Texture> m_textures;
   std::vector<std::unique_ptr<CommandPool>> m_commandPools;

   std::unique_ptr<DescriptorPool> m_descPool;
   std::unique_ptr<Swapchain> m_swapchain;
   std::unique_ptr<RenderPassStash> m_renderPasses;
   std::unique_ptr<SamplerStash> m_samplers;
   std::unique_ptr<PipelineStash> m_pipelines;

   struct QueueFamily
   {
      std::vector<VkQueue> queues;
      uint32_t index           = 0;
      uint32_t queueCount      = 0;
      CYD::QueueUsageFlag type = 0;
      bool supportsPresent     = false;
   };
   std::vector<QueueFamily> m_queueFamilies;

   // Extensions used to create the device
   const std::vector<const char*>& m_extensions;

   VkDevice m_vkDevice           = nullptr;
   VkPhysicalDevice m_physDevice = nullptr;
   std::unique_ptr<VkPhysicalDeviceProperties> m_physProps;
};
}
