#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <array>
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
FWDHANDLE( VkDescriptorSet );
FWDHANDLE( VkFramebuffer );

namespace vk
{
class Device;
class CommandPool;
class Swapchain;
class Buffer;
class Texture;
}

namespace CYD
{
struct PipelineInfo;
struct GraphicsPipelineInfo;
struct ComputePipelineInfo;
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
   void acquire( const Device& device, const CommandPool& pool, CYD::QueueUsageFlag usage );
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
   void bindVertexBuffer( Buffer* vertexBuf ) const;
   void bindIndexBuffer( Buffer* indexBuf, CYD::IndexType type ) const;
   void bindPipeline( const CYD::GraphicsPipelineInfo& info );
   void bindPipeline( const CYD::ComputePipelineInfo& info );
   void bindBuffer( Buffer* buffer, uint32_t set, uint32_t binding );
   void bindUniformBuffer( Buffer* buffer, uint32_t set, uint32_t binding );
   void bindTexture( Texture* texture, uint32_t set, uint32_t binding );
   void bindImage( Texture* texture, uint32_t set, uint32_t binding );
   void updatePushConstants( const CYD::PushConstantRange& range, const void* pData );

   // Render Pass
   // =============================================================================================
   void beginPass( Swapchain& swapchain, bool hasDepth );
   void beginPass(
       const CYD::RenderPassInfo& renderPassInfo,
       const std::vector<const Texture*>& textures );
   void endPass() const;

   // Dynamic State
   // =============================================================================================
   void setViewport( const CYD::Rectangle& viewport ) const;

   // Drawing
   // =============================================================================================
   void draw( size_t vertexCount );
   void drawIndexed( size_t indexCount );

   // Compute
   // =============================================================================================
   void dispatch( uint32_t workX, uint32_t workY, uint32_t workZ );

   // Transfers
   // =============================================================================================
   void copyBuffer( const Buffer* src, const Buffer* dst ) const;
   void uploadBufferToTex( const Buffer* src, Texture* dst ) const;

  private:
   // The updating and binding of descriptor sets is deferred all the way until we do a draw call.
   // We will only update and bind the "new" descriptor sets in Vulkan with the descriptor sets that
   // were bound to the command buffer since the last draw call. This ensures that we do not create
   // unnecessary descriptor sets and that we only update and bind what is needed for the next draw.
   VkDescriptorSet _findOrAllocateDescSet( size_t prevSize, uint32_t set );
   void _prepareDescriptorSets( CYD::PipelineType pipType );

   const Device* m_pDevice    = nullptr;
   const CommandPool* m_pPool = nullptr;

   // Info on the currently bound pipeline
   std::optional<VkPipeline> m_boundPip;
   std::optional<VkPipelineLayout> m_boundPipLayout;
   std::optional<VkRenderPass> m_boundRenderPass;

   // To keep in scope for destruction
   std::vector<VkFramebuffer> m_curFramebuffers;

   // MSVC's implementation of the move assignment/constructor for unordered_map is not noexcept.
   // This prevents us from using unordered_maps (from ShaderConstants) in STL containers properly.
   // Wtf why.
   std::unique_ptr<CYD::PipelineInfo> m_boundPipInfo;

   // Currently bound descriptor sets
   static constexpr uint32_t MAX_BOUND_DESCRIPTOR_SETS = 32;
   std::array<VkDescriptorSet, MAX_BOUND_DESCRIPTOR_SETS> m_boundSets;

   // All descriptor sets linked to this command buffer
   struct DescriptorSetInfo
   {
      uint32_t set;
      VkDescriptorSet vkDescSet;
   };
   std::vector<DescriptorSetInfo> m_descSets;

   // Used for deferred updating and binding of descriptor sets before a draw command
   struct BufferUpdateInfo
   {
      BufferUpdateInfo(
          const Buffer* buffer,
          CYD::ShaderResourceType type,
          uint32_t set,
          uint32_t binding )
          : buffer( buffer ), type( type ), set( set ), binding( binding )
      {
      }
      const Buffer* buffer;
      CYD::ShaderResourceType type;
      uint32_t set;
      uint32_t binding;
   };
   std::vector<BufferUpdateInfo> m_buffersToUpdate;

   struct TextureUpdateInfo
   {
      TextureUpdateInfo(
          const Texture* texture,
          CYD::ShaderResourceType type,
          uint32_t set,
          uint32_t binding )
          : texture( texture ), type( type ), set( set ), binding( binding )
      {
      }
      const Texture* texture;
      CYD::ShaderResourceType type;
      uint32_t set;
      uint32_t binding;
   };
   std::vector<TextureUpdateInfo> m_texturesToUpdate;

   // Syncing
   std::vector<VkSemaphore> m_semsToWait;
   std::vector<VkSemaphore> m_semsToSignal;

   CYD::QueueUsageFlag m_usage   = CYD::QueueUsage::UNKNOWN;
   bool m_isRecording            = false;
   bool m_wasSubmitted           = false;
   VkCommandBuffer m_vkCmdBuffer = nullptr;
   VkFence m_vkFence             = nullptr;
};
}
