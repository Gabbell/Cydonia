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
class CommandPool
{
  public:
   CommandPool( const Device& device, uint32_t familyIndex, QueueUsageFlag usage );
   ~CommandPool();

   const VkCommandPool& getVKCommandPool() const { return _vkPool; }

   std::shared_ptr<CommandBuffer> createCommandBuffer();

   QueueUsageFlag getType() const noexcept { return _type; }
   uint32_t getFamilyIndex() const noexcept { return _familyIndex; }

   void cleanup();

  private:
   const Device& _device;

   std::vector<std::shared_ptr<CommandBuffer>> _buffers;
   VkCommandPool _vkPool = nullptr;

   uint32_t _familyIndex;
   QueueUsageFlag _type;
};
}
