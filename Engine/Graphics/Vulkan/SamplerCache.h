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
class SamplerCache final
{
  public:
   SamplerCache( const Device& device );
   ~SamplerCache();

   const VkSampler findOrCreate( const CYD::SamplerInfo& info );

  private:
   const Device& m_device;
   std::unordered_map<CYD::SamplerInfo, VkSampler> m_samplers;
};
}
