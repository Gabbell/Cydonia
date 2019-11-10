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
FWDHANDLE( VkDescriptorSetLayout );

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

   const VkDescriptorSetLayout findOrCreate( const DescriptorSetLayoutInfo& info );
   const VkPipeline findOrCreate( const PipelineInfo& info );
   const VkPipelineLayout findOrCreate( const PipelineLayoutInfo& info );

  private:
   const Device& _device;

   std::unique_ptr<ShaderStash> _shaderStash;

   std::unordered_map<DescriptorSetLayoutInfo, VkDescriptorSetLayout> _descSetLayouts;
   std::unordered_map<PipelineLayoutInfo, VkPipelineLayout> _pipLayouts;
   std::unordered_map<PipelineInfo, VkPipeline> _pipelines;
};
}
