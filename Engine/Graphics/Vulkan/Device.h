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
class PipelineCache;
class RenderPassCache;
class SamplerCache;
class CommandPoolManager;
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
   NON_COPIABLE( Device );
   ~Device();

   // Interface
   // =============================================================================================
   Swapchain* createSwapchain( const CYD::SwapchainInfo& scInfo );
   CommandBuffer* createCommandBuffer(
       CYD::QueueUsageFlag usage,
       const std::string_view name,
       bool presentable = false );

   // Buffer creation function
   Buffer* createVertexBuffer( size_t size, const std::string_view name );
   Buffer* createIndexBuffer( size_t size, const std::string_view name );
   Buffer* createUniformBuffer( size_t size, const std::string_view name );
   Buffer* createBuffer( size_t size, const std::string_view name );
   Buffer* createStagingBuffer( size_t size );
   Texture* createTexture( const CYD::TextureDescription& desc );

   void cleanup();  // Clean up unused resources
   void clearPipelines();

   void waitUntilIdle();  // CPU wait until device is done working
   void waitOnFrame( uint32_t currentFrame );

   // Getters
   VkPhysicalDevice getPhysicalDevice() const noexcept { return m_physDevice; }
   VkDevice getVKDevice() const noexcept { return m_vkDevice; }

   Swapchain& getSwapchain() const { return *m_swapchain; }

   PipelineCache& getPipelineCache() const { return *m_pipelines; }
   RenderPassCache& getRenderPassCache() const { return *m_renderPasses; }
   SamplerCache& getSamplerCache() const { return *m_samplers; }
   DescriptorPool& getDescriptorPool() const { return *m_descPool; }

   // Queues
   struct QueueFamily
   {
      std::vector<VkQueue> queues;
      uint32_t queueCount      = 0;
      CYD::QueueUsageFlag type = 0;
      bool supportsPresent     = false;
   };

   const QueueFamily& getQueueFamilyFromIndex( uint32_t familyIndex ) const;
   uint32_t getQueueFamilyIndexFromUsage(
       CYD::QueueUsageFlag usage,
       bool supportsPresentation = false ) const;
   VkQueue getQueueFromUsage( CYD::QueueUsageFlag usage, bool supportsPresentation = false ) const;

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

   // Common buffer function
   Buffer* _createBuffer(
       size_t size,
       CYD::BufferUsageFlag usage,
       CYD::MemoryTypeFlag memoryType,
       const std::string_view name );

   // =============================================================================================
   // Private Members
   // =============================================================================================
   const CYD::Window& m_window;

   const Instance& m_instance;
   const Surface& m_surface;

   std::vector<Buffer> m_buffers;
   std::vector<Texture> m_textures;
   std::unique_ptr<CommandPoolManager> m_commandPoolManager;

   std::unique_ptr<DescriptorPool> m_descPool;
   std::unique_ptr<Swapchain> m_swapchain;
   std::unique_ptr<RenderPassCache> m_renderPasses;
   std::unique_ptr<SamplerCache> m_samplers;
   std::unique_ptr<PipelineCache> m_pipelines;

   std::vector<QueueFamily> m_queueFamilies;

   // Extensions used to create the device
   const std::vector<const char*>& m_extensions;

   VkDevice m_vkDevice           = nullptr;
   VkPhysicalDevice m_physDevice = nullptr;
   std::unique_ptr<VkPhysicalDeviceProperties> m_physProps;
};
}
