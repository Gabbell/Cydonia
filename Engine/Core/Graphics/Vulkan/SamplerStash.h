#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkSampler );

namespace cyd
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class SamplerStash
{
  public:
   SamplerStash( const Device& device );
   ~SamplerStash();

   const VkSampler findOrCreate( const SamplerInfo& info );

  private:
   const Device& _device;
   std::unordered_map<SamplerInfo, VkSampler> _samplers;
};
}
