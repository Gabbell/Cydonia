#pragma once
#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/PipelineInfos.h>

#include <array>
#include <atomic>
#include <memory>
#include <optional>
#include <unordered_set>

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
FWDHANDLE( VkSampler );
FWDHANDLE( VkBuffer );
FWDHANDLE( VkImage );
FWDHANDLE( VkImageView );
FWDFLAG( VkPipelineStageFlags );

enum VkDescriptorType : int;
enum VkImageLayout : int;

namespace vk
{
class Device;
class CommandBufferPool;
class Swapchain;
class Buffer;
class Texture;
struct DescriptorSetInfo;
}

namespace CYD
{
struct PipelineInfo;
struct GraphicsPipelineInfo;
struct ComputePipelineInfo;
class Framebuffer;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class CommandBuffer final
{
  public:
   CommandBuffer();
   MOVABLE( CommandBuffer );
   ~CommandBuffer();

   // Allocation and Deallocation
   // =============================================================================================
   void acquire(
       const Device& device,
       const CommandBufferPool& pool,
       CYD::QueueUsageFlag usage,
       const std::string_view name );
   void free();     // Releases resources attached to this command buffer
   void release();  // Destroys the sync objects linked to this command buffer

   bool inUse() const { return ( *m_useCount ) > 0; }

   void incUse();
   void decUse();

   // Getters
   // =============================================================================================
   VkPipelineStageFlags getWaitStages() const noexcept;
   VkCommandBuffer getVKBuffer() const noexcept { return m_vkCmdBuffer; }
   VkFence getVKFence() const noexcept { return m_vkFence; }

   // Returns the semaphore that will be signaled when this command buffer is done execution
   VkSemaphore getDoneSemaphore() const;

   const std::string_view getName() const { return m_name; }

   // Status
   // =============================================================================================
   bool isReleased() const noexcept { return m_state == State::RELEASED; }
   bool isFree() const noexcept { return m_state == State::FREE; }
   bool isSubmitted() const noexcept { return m_state == State::SUBMITTED; }
   bool isCompleted();
   void waitForCompletion() const;  // CPU Spinlock, avoid calling this

   // Synchronization
   void syncOnCommandList( CommandBuffer* cmdBufferToWaitOn );
   void syncOnSwapchain( const Swapchain* swapchain );
   void syncToSwapchain( const Swapchain* swapchain );

   // Recording and Submission
   // =============================================================================================
   void startRecording();
   void endRecording();
   void submit();
   void reset();

   // Bindings
   // =============================================================================================
   void bindVertexBuffer( Buffer* vertexBuf );
   void bindIndexBuffer( Buffer* indexBuf, CYD::IndexType type, uint32_t offset );

   void bindPipeline( const CYD::GraphicsPipelineInfo& info );
   void bindPipeline( const CYD::ComputePipelineInfo& info );

   void bindSwapchainColor(
       Swapchain& swapchain,
       CYD::ShaderResourceType type,
       uint8_t binding,
       uint8_t set );
   void bindSwapchainDepth(
       Swapchain& swapchain,
       CYD::ShaderResourceType type,
       uint8_t binding,
       uint8_t set );
   void bindTexture( Texture* texture, uint8_t binding, uint8_t set );
   void
   bindTexture( Texture* texture, const CYD::SamplerInfo& sampler, uint8_t binding, uint8_t set );
   void bindImage( Texture* texture, uint8_t binding, uint8_t set );
   void bindBuffer( Buffer* buffer, uint8_t binding, uint8_t set, uint32_t offset, uint32_t range );
   void bindUniformBuffer(
       Buffer* buffer,
       uint8_t binding,
       uint8_t set,
       uint32_t offset,
       uint32_t range );

   void updatePushConstants( const CYD::PushConstantRange& range, const void* pData );

   // Rendering scope
   // =============================================================================================
   void beginRendering( Swapchain& swapchain );
   void beginRendering(
       const CYD::Framebuffer& fb,
       const CYD::RenderPassInfo& info,
       const std::vector<Texture*>& targets );
   void nextPass() const;
   void endRendering();

   // Dynamic State
   // =============================================================================================
   void setViewport( const CYD::Viewport& viewport ) const;
   void setScissor( const CYD::Rectangle& scissor ) const;

   // Drawing
   // =============================================================================================
   void draw( size_t vertexCount, size_t instanceCount, size_t firstVertex, size_t firstInstance );
   void
   drawIndexed( size_t indexCount, size_t instanceCount, size_t firstIndex, size_t firstInstance );

   // Compute
   // =============================================================================================
   void dispatch( uint32_t workX, uint32_t workY, uint32_t workZ );

   // Transfers
   // =============================================================================================
   void copyBuffer( Buffer* src, Buffer* dst );
   void uploadBufferToTex( Buffer* src, Texture* dst );

   // Debug
   // ==============================================================================================
   void beginDebugRange( const char* name, const std::array<float, 4>& color );
   void endDebugRange();
   void insertDebugMarker( const char* name, const std::array<float, 4>& color );

  private:
   // Constants
   // =============================================================================================
   static constexpr char DEFAULT_CMDBUFFER_NAME[]      = "Unknown Command Buffer";
   static constexpr uint32_t MAX_BOUND_DESCRIPTOR_SETS = 8;

   // Command Buffer Internal State
   // =============================================================================================
   enum State
   {
      ACQUIRED = 0,  // Has been acquired by the command pool
      RECORDING,     // Has commands being recorded
      STANDBY,       // Waiting to be submitted
      SUBMITTED,     // Has been submitted
      COMPLETED,     // Is completed
      FREE,          // Has done execution but is still holding sync objects
      RELEASED,      // Is completely free of resources and sync objects
      LIMBO,         // Something horrible happened
      NUMBER_OF_STATES
   };

   bool _isValidStateTransition( State desiredState );

   // Descriptor Sets and Binding
   // =============================================================================================

   // The updating and binding of descriptor sets is deferred all the way until we do a draw call.
   // We will only update and bind the "new" descriptor sets in Vulkan with the descriptor sets that
   // were bound to the command buffer since the last draw call. This ensures that we do not create
   // unnecessary descriptor sets and that we only update and bind what is needed for the next draw.
   VkDescriptorSet _findOrAllocateDescSet( size_t prevSize, uint8_t set );
   void _prepareDescriptorSets();

   // Dependencies
   // =============================================================================================
   // Used to keep track of the resources dependent on this command buffer to potentially release
   // them once the command buffer is done
   template <class T>
   void _addDependency( T* dependency );

   void _setRenderArea( int offsetX, int offsetY, uint32_t width, uint32_t height );

   // Member Variables
   // =============================================================================================
   const Device* m_pDevice          = nullptr;
   const CommandBufferPool* m_pPool = nullptr;

   State m_state = State::RELEASED;

   std::string_view m_name = DEFAULT_CMDBUFFER_NAME;

   // Info on the currently bound pipeline
   VkPipeline m_boundPip             = nullptr;
   VkPipelineLayout m_boundPipLayout = nullptr;

   VkRenderPass m_boundRenderPass = nullptr;
   CYD::Rectangle m_renderArea;
   std::optional<CYD::RenderPassInfo> m_boundRenderPassInfo;
   std::vector<Texture*> m_targets;
   uint32_t m_currentSubpass = 0;

   // To keep in scope for destruction
   std::vector<VkFramebuffer> m_curFramebuffers;

   // MSVC's implementation of the move assignment/constructor for unordered_map is not noexcept.
   // This prevents us from using unordered_maps (from ShaderConstants) in STL containers properly.
   // Wtf why.
   std::unique_ptr<CYD::PipelineInfo> m_boundPipInfo;

   // Descriptor sets to bind
   std::array<VkDescriptorSet, MAX_BOUND_DESCRIPTOR_SETS> m_setsToBind = {};

   // All descriptor sets whose lifetime is linked to this command buffer
   struct DescriptorSetInfo
   {
      uint8_t set;
      VkDescriptorSet vkDescSet;
   };
   std::vector<DescriptorSetInfo> m_descSets;

   // Used for deferred updating and binding of descriptor sets before a draw command
   using BufferBinding = CYD::FlatShaderBinding<Buffer>;

   struct TextureBinding
   {
      VkDescriptorType type;
      VkImageView imageView;
      VkImageLayout layout;
      uint32_t binding;
      uint32_t set;
   };

   std::vector<BufferBinding> m_buffersToUpdate;
   std::vector<TextureBinding> m_texturesToUpdate;
   std::vector<VkSampler> m_samplers;

   // Resource Dependencies
   std::unordered_set<CommandBuffer*> m_cmdBuffersInUse;
   std::unordered_set<Texture*> m_texturesInUse;
   std::unordered_set<Buffer*> m_buffersInUse;

   // Syncing
   std::unique_ptr<std::atomic<uint32_t>> m_useCount;
   std::vector<VkPipelineStageFlags> m_stagesToWait;
   std::vector<VkSemaphore> m_semsToWait;
   std::vector<VkSemaphore> m_semsToSignal;

   CYD::QueueUsageFlag m_usage   = CYD::QueueUsage::UNKNOWN;
   VkCommandBuffer m_vkCmdBuffer = nullptr;
   VkFence m_vkFence             = nullptr;
   VkSampler m_defaultSampler    = nullptr;
};
}
