#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Types.h>

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
   CommandPool( const Device& device, uint32_t familyIndex, UsageFlag usage );
   ~CommandPool();

   const VkCommandPool& getVKCommandPool() const { return _vkPool; }

   std::shared_ptr<CommandBuffer> createCommandBuffer();

   UsageFlag getType() const { return _type; }

   void cleanup();

  private:
   const Device& _device;

   std::vector<std::shared_ptr<CommandBuffer>> _buffers;
   VkCommandPool _vkPool = nullptr;

   UsageFlag _type;
};
}
