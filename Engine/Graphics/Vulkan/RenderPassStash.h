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
class RenderPassStash final
{
  public:
   explicit RenderPassStash( const Device& device );
   ~RenderPassStash();

   VkRenderPass findOrCreate( const CYD::RenderPassInfo& info );

  private:
   void _createDefaultRenderPasses();

   const Device& m_device;

   std::unordered_map<CYD::RenderPassInfo, VkRenderPass> m_renderPasses;
};
}
