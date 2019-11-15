#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

#include <memory>
#include <optional>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkCommandBuffer );
FWDHANDLE( VkFence );
FWDHANDLE( VkSemaphore );
FWDHANDLE( VkPipeline );
FWDHANDLE( VkPipelineLayout );
FWDHANDLE( VkRenderPass );
namespace cyd
{
class CommandPool;
class Device;
class Swapchain;
class Buffer;
class Texture;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class CommandBuffer
{
  public:
   CommandBuffer( const Device& device, const CommandPool& pool, QueueUsageFlag usage );
   ~CommandBuffer();

   // Getters
   // =============================================================================================
   const VkCommandBuffer& getVKBuffer() const { return _vkCmdBuffer; }
   const VkFence& getVKFence() const { return _vkFence; }

   // Status
   // =============================================================================================
   bool isCompleted() const;
   bool wasSubmitted() const noexcept { return _wasSubmitted; }
   void waitForCompletion() const;  // CPU Spinlock, avoid calling this

   // Recording and Submission
   // =============================================================================================
   void startRecording();
   void endRecording();
   void submit();

   // Bindings
   // =============================================================================================
   void updatePushConstants( const PushConstantRange& range, void* data );
   void bindPipeline( const PipelineInfo& info );
   void bindVertexBuffer( const std::shared_ptr<Buffer> vertexBuf );
   void bindBuffer( const std::shared_ptr<Buffer> buffer );
   void bindTexture( const std::shared_ptr<Texture> texture );

   // Render Pass
   // =============================================================================================
   void beginPass( Swapchain* swapchain );
   void endPass();

   // Dynamic State
   // =============================================================================================
   void setViewport( uint32_t width, uint32_t height );

   // Drawing
   // =============================================================================================
   void draw( uint32_t vertexCount );

   // Transfers
   // =============================================================================================
   void copyBuffer( const std::shared_ptr<Buffer> src, const std::shared_ptr<Buffer> dst );
   void uploadBufferToTex( const std::shared_ptr<Buffer> src, const std::shared_ptr<Texture> dst );

  private:
   const Device& _device;
   const CommandPool& _pool;

   // Info on the currently bound pipeline
   std::optional<VkPipeline> _boundPip;
   std::optional<VkPipelineLayout> _boundPipLayout;
   std::optional<VkRenderPass> _boundRenderPass;

   // Syncing
   std::vector<VkSemaphore> _semsToWait;
   std::vector<VkSemaphore> _semsToSignal;

   QueueUsageFlag _usage;
   bool _isRecording            = false;
   bool _wasSubmitted           = false;
   VkCommandBuffer _vkCmdBuffer = nullptr;
   VkFence _vkFence             = nullptr;
};
}
