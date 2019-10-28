#pragma once

#include <Core/Common/Include.h>

#include <Core/Graphics/Vulkan/Types.h>

#include <memory>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkPipeline );
FWDHANDLE( VkPipelineLayout );

namespace cyd
{
class Device;
class ShaderStash;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class PipelineStash
{
  public:
   explicit PipelineStash( const Device& device );
   ~PipelineStash();

   const VkPipeline findOrCreate( const PipelineInfo& info );

  private:
   const Device& _device;

   std::unique_ptr<ShaderStash> _shaderStash;

   std::unordered_map<PipelineInfo, VkPipeline> _pipelines;
   std::unordered_map<PipelineLayoutInfo, VkPipelineLayout> _pipLayouts;
};
}
