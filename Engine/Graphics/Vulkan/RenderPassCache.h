#pragma once

#include <Common/Include.h>

#include <Graphics/Vulkan/VulkanTypes.h>

#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkRenderPass );

namespace vk
{
class Device;
}

// ================================================================================================
// Definitions
// ================================================================================================
namespace vk
{
class RenderPassCache final
{
  public:
   explicit RenderPassCache( const Device& device );
   ~RenderPassCache();

   VkRenderPass findOrCreate( const RenderPassInfo& targetsInfo );

  private:
   void _createDefaultRenderPasses();

   const Device& m_device;

   std::unordered_map<RenderPassInfo, VkRenderPass> m_renderPasses;
};
}
