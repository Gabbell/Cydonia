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

   const VkCommandPool& getVKCommandPool() const { return _vkPool; }

   CommandBuffer* createCommandBuffer();
 
   cyd::QueueUsageFlag getType() const noexcept { return _type; }
   uint32_t getFamilyIndex() const noexcept { return _familyIndex; }
   bool supportsPresentation() const noexcept { return _supportsPresentation; }

  private:
   void _createCommandPool();

   const Device& _device;

   // Command Buffer Pool
   static constexpr uint32_t MAX_CMD_BUFFERS_IN_FLIGHT = 5;
   std::vector<CommandBuffer> _cmdBuffers;

   cyd::HandleManager _cmdBufferHandles;

   VkCommandPool _vkPool = nullptr;

   cyd::QueueUsageFlag _type;
   uint32_t _familyIndex      = 0;
   bool _supportsPresentation = false;
};
}
