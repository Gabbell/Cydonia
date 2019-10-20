#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Types.h>

#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkRenderPass );
namespace cyd
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class RenderPassStash
{
  public:
   explicit RenderPassStash( const Device& device );
   ~RenderPassStash();

   const VkRenderPass findOrCreate( const RenderPassInfo& info );

  private:
   void _createDefaultRenderPasses();

   const Device& _device;

   std::unordered_map<RenderPassInfo, VkRenderPass> _renderPasses;
};
}
