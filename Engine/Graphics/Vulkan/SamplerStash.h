#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkSampler );

namespace vk
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class SamplerStash final
{
  public:
   SamplerStash( const Device& device );
   ~SamplerStash();

   const VkSampler findOrCreate( const cyd::SamplerInfo& info );

  private:
   const Device& m_device;
   std::unordered_map<cyd::SamplerInfo, VkSampler> m_samplers;
};
}
