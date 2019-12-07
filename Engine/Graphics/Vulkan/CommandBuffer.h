#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

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

namespace vk
{
class Device;
class CommandPool;
class Swapchain;
class Buffer;
class Texture;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class CommandBuffer final
{
  public:
   CommandBuffer() = default;
   MOVABLE( CommandBuffer );
   ~CommandBuffer() = default;

   // Allocation and Deallocation
   // =============================================================================================
   void seize( const Device& device, const CommandPool& pool, cyd::QueueUsageFlag usage );
   void release();

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
   void bindVertexBuffer( const Buffer* vertexBuf );
   template <typename T>
   void bindIndexBuffer( const Buffer* indexBuf );
   void bindPipeline( const cyd::PipelineInfo& info );
   void bindBuffer( const Buffer* buffer );
   void bindTexture( Texture* texture );
   void updatePushConstants( const cyd::PushConstantRange& range, void* data );

   // Render Pass
   // =============================================================================================
   void beginPass( Swapchain& swapchain );
   void endPass();

   // Dynamic State
   // =============================================================================================
   void setViewport( const cyd::Rectangle& viewport );

   // Drawing
   // =============================================================================================
   void draw( size_t vertexCount );
   void drawIndexed( size_t indexCount );

   // Transfers
   // =============================================================================================
   void copyBuffer( const Buffer* src, const Buffer* dst );
   void uploadBufferToTex( const Buffer* src, Texture* dst );

  private:
   const Device* _device    = nullptr;
   const CommandPool* _pool = nullptr;

   // Info on the currently bound pipeline
   std::optional<VkPipeline> _boundPip;
   std::optional<VkPipelineLayout> _boundPipLayout;
   std::optional<VkRenderPass> _boundRenderPass;
   std::optional<cyd::PipelineInfo> _boundPipInfo;

   // Syncing
   std::vector<VkSemaphore> _semsToWait;
   std::vector<VkSemaphore> _semsToSignal;

   cyd::QueueUsageFlag _usage;
   bool _isRecording            = false;
   bool _wasSubmitted           = false;
   VkCommandBuffer _vkCmdBuffer = nullptr;
   VkFence _vkFence             = nullptr;
};
}
