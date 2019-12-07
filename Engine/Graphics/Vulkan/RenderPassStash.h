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

   const VkRenderPass findOrCreate( const cyd::RenderPassInfo& info );

  private:
   void _createDefaultRenderPasses();

   const Device& _device;

   std::unordered_map<cyd::RenderPassInfo, VkRenderPass> _renderPasses;
};
}
