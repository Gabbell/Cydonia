#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

#include <memory>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkPipeline );
FWDHANDLE( VkRenderPass );
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
   const VkPipeline findOrCreate( const cyd::PipelineInfo& info, VkRenderPass renderPass );
   const VkPipelineLayout findOrCreate( const cyd::PipelineLayoutInfo& info );

  private:
   const Device& m_device;

   std::unique_ptr<ShaderStash> m_shaderStash;

   std::unordered_map<cyd::DescriptorSetLayoutInfo, VkDescriptorSetLayout> m_descSetLayouts;
   std::unordered_map<cyd::PipelineLayoutInfo, VkPipelineLayout> m_pipLayouts;
   std::unordered_map<cyd::PipelineInfo, VkPipeline> m_pipelines;
};
}
