#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Types.h>

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
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class CommandBuffer
{
  public:
   CommandBuffer( const Device& device, const CommandPool& pool, UsageFlag usage );
   ~CommandBuffer();

   const VkCommandBuffer& getVKBuffer() const { return _vkBuffer; }
   const VkFence& getVKFence() const { return _vkFence; }

   bool isCompleted() const;

   void startRecording();
   void endRecording();

   void setPipeline( const PipelineInfo& info );
   void setViewport( uint32_t width, uint32_t height );
   void beginPass( Swapchain* swapchain );
   void draw();
   void endPass();
   void submit();

  private:
   const Device& _device;
   const CommandPool& _pool;

   // Info on the currently bound pipeline
   std::optional<PipelineInfo> _boundPipeline;

   UsageFlag _usage;
   bool _isRecording       = false;
   VkCommandBuffer _vkBuffer = nullptr;
   VkFence _vkFence          = nullptr;
};
}
