#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <cstdint>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkCommandPool );

namespace vk
{
class Device;
class CommandBuffer;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class CommandBufferPool final
{
  public:
   CommandBufferPool() = delete;
   CommandBufferPool(
       const Device& device,
       uint32_t familyIndex,
       CYD::QueueUsageFlag usage,
       bool supportsPresentation );
   MOVABLE( CommandBufferPool );
   ~CommandBufferPool();

   void waitUntilDone();

   void cleanup();

   const VkCommandPool& getVKCommandPool() const { return m_vkPool; }

   CommandBuffer* createCommandBuffer( CYD::QueueUsageFlag usage, const std::string_view name );

   CYD::QueueUsageFlag getType() const noexcept { return m_type; }
   uint32_t getFamilyIndex() const noexcept { return m_familyIndex; }
   bool supportsPresentation() const noexcept { return m_supportsPresentation; }

  private:
   void _createCommandBufferPool();

   const Device* m_pDevice = nullptr;

   // Command Buffer Pool
   static constexpr uint32_t MAX_CMD_BUFFERS_IN_FLIGHT = 16;
   std::vector<CommandBuffer> m_cmdBuffers;
   uint32_t m_cmdBuffersInUse = 0;

   VkCommandPool m_vkPool = nullptr;

   CYD::QueueUsageFlag m_type;
   uint32_t m_familyIndex      = 0;
   bool m_supportsPresentation = false;
};
}
