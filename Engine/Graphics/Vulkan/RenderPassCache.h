#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

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
// Definition
// ================================================================================================
namespace vk
{
class RenderPassCache final
{
  public:
   explicit RenderPassCache( const Device& device );
   ~RenderPassCache();

   VkRenderPass findOrCreate( const CYD::FramebufferInfo& targetsInfo );

  private:
   void _createDefaultRenderPasses();

   const Device& m_device;

   std::unordered_map<CYD::FramebufferInfo, VkRenderPass> m_renderPasses;
};
}
