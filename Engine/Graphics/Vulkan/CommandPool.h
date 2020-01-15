#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Vulkan/CommandBuffer.h>
#include <Handles/HandleManager.h>

#include <cstdint>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkCommandPool );

namespace vk
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class CommandPool final
{
  public:
   CommandPool(
       const Device& device,
       uint32_t familyIndex,
       cyd::QueueUsageFlag usage,
       bool supportsPresentation );
   ~CommandPool();

   const VkCommandPool& getVKCommandPool() const { return m_vkPool; }

   CommandBuffer* createCommandBuffer();
 
   cyd::QueueUsageFlag getType() const noexcept { return m_type; }
   uint32_t getFamilyIndex() const noexcept { return m_familyIndex; }
   bool supportsPresentation() const noexcept { return m_supportsPresentation; }

  private:
   void _createCommandPool();

   const Device& m_device;

   // Command Buffer Pool
   static constexpr uint32_t MAX_CMD_BUFFERS_IN_FLIGHT = 5;
   std::vector<CommandBuffer> m_cmdBuffers;

   cyd::HandleManager m_cmdBufferHandles;

   VkCommandPool m_vkPool = nullptr;

   cyd::QueueUsageFlag m_type;
   uint32_t m_familyIndex      = 0;
   bool m_supportsPresentation = false;
};
}