#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <memory>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkPipeline );
FWDHANDLE( VkPipelineLayout );
FWDHANDLE( VkDescriptorSetLayout );

namespace vk
{
class Device;
class ShaderStash;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class PipelineStash final
{
  public:
   explicit PipelineStash( const Device& device );
   ~PipelineStash();

   const VkDescriptorSetLayout findOrCreate( const cyd::DescriptorSetLayoutInfo& info );
   const VkPipeline findOrCreate( const cyd::PipelineInfo& info );
   const VkPipelineLayout findOrCreate( const cyd::PipelineLayoutInfo& info );

  private:
   const Device& _device;

   std::unique_ptr<ShaderStash> _shaderStash;

   std::unordered_map<cyd::DescriptorSetLayoutInfo, VkDescriptorSetLayout> _descSetLayouts;
   std::unordered_map<cyd::PipelineLayoutInfo, VkPipelineLayout> _pipLayouts;
   std::unordered_map<cyd::PipelineInfo, VkPipeline> _pipelines;
};
}
