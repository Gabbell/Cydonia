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
namespace cyd
{
class CommandPool;
class Device;
class Swapchain;
class Buffer;
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

   const VkCommandBuffer& getVKBuffer() const { return _vkCmdBuffer; }
   const VkFence& getVKFence() const { return _vkFence; }

   bool isCompleted() const;
   void waitForCompletion() const;  // CPU Spinlock, avoid calling this

   void startRecording();
   void endRecording();

   void pushConstants( const PipelineLayoutInfo& info, ShaderStage stage );
   void bindPipeline( const PipelineInfo& info );
   void bindVertexBuffer( const std::shared_ptr<Buffer> vertexBuf );
   void setViewport( uint32_t width, uint32_t height );
   void beginPass( Swapchain* swapchain );
   void draw();
   void endPass();
   void copyBuffer( const std::shared_ptr<Buffer> src, const std::shared_ptr<Buffer> dst );
   void submit();

  private:
   const Device& _device;
   const CommandPool& _pool;

   // Info on the currently bound pipeline
   std::optional<PipelineInfo> _boundPip;
   std::optional<PipelineLayoutInfo> _boundPipLayout;

   QueueUsageFlag _usage;
   bool _isRecording            = false;
   VkCommandBuffer _vkCmdBuffer = nullptr;
   VkFence _vkFence             = nullptr;
};
}
