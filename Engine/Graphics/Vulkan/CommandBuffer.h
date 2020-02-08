#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <optional>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkCommandBuffer );
FWDHANDLE( VkFence );
FWDHANDLE( VkSemaphore );
FWDHANDLE( VkPipeline );
FWDHANDLE( VkPipelineLayout );
FWDHANDLE( VkRenderPass );
FWDHANDLE( VkDescriptorSet );

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
   MOVABLE( CommandBuffer )
   ~CommandBuffer() = default;

   // Allocation and Deallocation
   // =============================================================================================
   void acquire( const Device& device, const CommandPool& pool, cyd::QueueUsageFlag usage );
   void release();

   // Getters
   // =============================================================================================
   const VkCommandBuffer& getVKBuffer() const { return m_vkCmdBuffer; }
   const VkFence& getVKFence() const { return m_vkFence; }

   // Status
   // =============================================================================================
   bool isCompleted() const;
   bool wasSubmitted() const noexcept { return m_wasSubmitted; }
   void waitForCompletion() const;  // CPU Spinlock, avoid calling this

   // Recording and Submission
   // =============================================================================================
   void startRecording();
   void endRecording();
   void submit();
   void reset();

   // Bindings
   // =============================================================================================
   void bindVertexBuffer( const Buffer* vertexBuf ) const;
   template <typename T>
   void bindIndexBuffer( const Buffer* indexBuf );
   void bindPipeline( const cyd::PipelineInfo& info );
   void bindBuffer( const Buffer* buffer, uint32_t set, uint32_t binding );
   void bindTexture( const Texture* texture, uint32_t set, uint32_t binding );
   void updatePushConstants( const cyd::PushConstantRange& range, const void* pData );

   // Render Pass
   // =============================================================================================
   void beginPass( const cyd::RenderPassInfo& renderPassInfo, Swapchain& swapchain );
   void endPass() const;

   // Dynamic State
   // =============================================================================================
   void setViewport( const cyd::Rectangle& viewport ) const;

   // Drawing
   // =============================================================================================
   void draw( size_t vertexCount );
   void drawIndexed( size_t indexCount );

   // Transfers
   // =============================================================================================
   void copyBuffer( const Buffer* src, const Buffer* dst ) const;
   void uploadBufferToTex( const Buffer* src, Texture* dst );

  private:
   void _prepareDescriptorSets();

   const Device* m_pDevice    = nullptr;
   const CommandPool* m_pPool = nullptr;

   // Info on the currently bound pipeline
   std::optional<cyd::PipelineInfo> m_boundPipInfo;
   std::optional<VkPipeline> m_boundPip;
   std::optional<VkPipelineLayout> m_boundPipLayout;
   std::optional<VkRenderPass> m_boundRenderPass;

   // The buffers/textures to update before the next draw along with a list of descriptor sets that
   // were allocated by this command buffer
   std::vector<VkDescriptorSet> m_descSets;

   // Syncing
   std::vector<VkSemaphore> m_semsToWait;
   std::vector<VkSemaphore> m_semsToSignal;

   cyd::QueueUsageFlag m_usage   = cyd::QueueUsage::UNKNOWN;
   cyd::ImageLayout m_prevLayout = cyd::ImageLayout::UNDEFINED;
   bool m_isRecording            = false;
   bool m_wasSubmitted           = false;
   VkCommandBuffer m_vkCmdBuffer = nullptr;
   VkFence m_vkFence             = nullptr;
};
}
