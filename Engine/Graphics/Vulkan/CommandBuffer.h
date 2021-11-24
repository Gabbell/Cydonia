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
FWDFLAG( VkPipelineStageFlags );

namespace vk
{
class Device;
class CommandBufferPool;
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
   void bindIndexBuffer( Buffer* indexBuf, CYD::IndexType type );

   void bindPipeline( const CYD::GraphicsPipelineInfo& info );
   void bindPipeline( const CYD::ComputePipelineInfo& info );

   void bindTexture( Texture* texture, uint32_t set, uint32_t binding );
   void bindImage( Texture* texture, uint32_t set, uint32_t binding );
   void bindBuffer( Buffer* buffer, uint32_t set, uint32_t binding );
   void bindUniformBuffer( Buffer* buffer, uint32_t set, uint32_t binding );

   void updatePushConstants( const CYD::PushConstantRange& range, const void* pData );

   // Rendering scope
   // =============================================================================================
   void beginRendering( Swapchain& swapchain );
   void beginRendering(
       const CYD::RenderTargetsInfo& targetsInfo,
       const std::vector<const Texture*>& targets );
   void nextPass() const;
   void endRendering() const;

   // Dynamic State
   // =============================================================================================
   void setViewport( const CYD::Viewport& viewport ) const;
   void setScissor( const CYD::Rectangle& scissor ) const;

   // Drawing
   // =============================================================================================
   void draw( size_t vertexCount, size_t firstVertex );
   void drawIndexed( size_t indexCount, size_t firstIndex );

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

   // Contains all the valid state transitions
   bool m_stateValidationTable[NUMBER_OF_STATES][NUMBER_OF_STATES] = {};

   void _initStateValidationTable();
   bool _isValidStateTransition( State desiredState );

   // Descriptor Sets and Binding
   // =============================================================================================
   // Used for deferred updating and binding of descriptor sets before a draw command
   using BufferBinding  = CYD::ResourceBinding<Buffer*>;
   using TextureBinding = CYD::ResourceBinding<Texture*>;

   // The updating and binding of descriptor sets is deferred all the way until we do a draw call.
   // We will only update and bind the "new" descriptor sets in Vulkan with the descriptor sets that
   // were bound to the command buffer since the last draw call. This ensures that we do not create
   // unnecessary descriptor sets and that we only update and bind what is needed for the next draw.
   VkDescriptorSet _findOrAllocateDescSet( size_t prevSize, uint32_t set );
   void _prepareDescriptorSets( CYD::PipelineType pipType );

   // Dependencies
   // =============================================================================================
   // Used to keep track of the resources dependent on this command buffer to potentially release
   // them once the command buffer is done
   template <class T>
   void _addDependency( T* dependency );

   // Member Variables
   // =============================================================================================
   const Device* m_pDevice          = nullptr;
   const CommandBufferPool* m_pPool = nullptr;

   State m_state = State::RELEASED;

   static constexpr char DEFAULT_CMDBUFFER_NAME[] = "Unknown Command Buffer Name";
   std::string_view m_name                        = DEFAULT_CMDBUFFER_NAME;

   // Info on the currently bound pipeline
   std::optional<VkPipeline> m_boundPip;
   std::optional<VkPipelineLayout> m_boundPipLayout;

   std::optional<VkRenderPass> m_boundRenderPass;
   std::optional<CYD::RenderTargetsInfo> m_boundTargetsInfo;
   uint32_t m_currentSubpass = 0;

   // To keep in scope for destruction
   std::vector<VkFramebuffer> m_curFramebuffers;

   // MSVC's implementation of the move assignment/constructor for unordered_map is not noexcept.
   // This prevents us from using unordered_maps (from ShaderConstants) in STL containers properly.
   // Wtf why.
   std::unique_ptr<CYD::PipelineInfo> m_boundPipInfo;

   // Currently bound descriptor sets
   static constexpr uint32_t MAX_BOUND_DESCRIPTOR_SETS                 = 8;
   std::array<VkDescriptorSet, MAX_BOUND_DESCRIPTOR_SETS> m_setsToBind = {};

   // All descriptor sets whose lifetime is linked to this command buffer
   struct DescriptorSetInfo
   {
      uint32_t set;
      VkDescriptorSet vkDescSet;
   };
   std::vector<DescriptorSetInfo> m_descSets;

   std::vector<BufferBinding> m_buffersToUpdate;
   std::vector<TextureBinding> m_texturesToUpdate;

   // Dependencies
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
