#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

#include <cstdint>
#include <memory>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkCommandPool );
namespace cyd
{
class Device;
class CommandBuffer;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class CommandPool final
{
  public:
   CommandPool(
       const Device& device,
       uint32_t familyIndex,
       QueueUsageFlag usage,
       bool supportsPresentation );
   ~CommandPool();

   const VkCommandPool& getVKCommandPool() const { return _vkPool; }

   std::shared_ptr<CommandBuffer> createCommandBuffer();

   QueueUsageFlag getType() const noexcept { return _type; }
   uint32_t getFamilyIndex() const noexcept { return _familyIndex; }
   bool supportsPresentation() const noexcept { return _supportsPresentation; }

   void cleanup();

  private:
   void _createCommandPool();

   const Device& _device;

   std::vector<std::shared_ptr<CommandBuffer>> _buffers;
   VkCommandPool _vkPool = nullptr;

   QueueUsageFlag _type;
   uint32_t _familyIndex      = 0;
   bool _supportsPresentation = false;
};
}
