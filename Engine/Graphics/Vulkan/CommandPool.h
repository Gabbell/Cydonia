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
class CommandPool final
{
  public:
   CommandPool() = delete;
   CommandPool(
       const Device& device,
       uint32_t familyIndex,
       cyd::QueueUsageFlag usage,
       bool supportsPresentation );
   MOVABLE( CommandPool )
   ~CommandPool();

   const VkCommandPool& getVKCommandPool() const { return m_vkPool; }

   CommandBuffer* createCommandBuffer();
 
   cyd::QueueUsageFlag getType() const noexcept { return m_type; }
   uint32_t getFamilyIndex() const noexcept { return m_familyIndex; }
   bool supportsPresentation() const noexcept { return m_supportsPresentation; }

  private:
   void _createCommandPool();

   const Device* m_pDevice = nullptr;

   // Command Buffer Pool
   static constexpr uint32_t MAX_CMD_BUFFERS_IN_FLIGHT = 5;
   std::vector<CommandBuffer> m_cmdBuffers;

   VkCommandPool m_vkPool = nullptr;

   cyd::QueueUsageFlag m_type;
   uint32_t m_familyIndex      = 0;
   bool m_supportsPresentation = false;
};
}
