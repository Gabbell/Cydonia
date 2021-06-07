#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/PipelineInfos.h>

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
   PipelineStash() = delete;
   NON_COPIABLE( PipelineStash );
   explicit PipelineStash( const Device& device );
   ~PipelineStash();

   VkDescriptorSetLayout findOrCreate( const CYD::ShaderSetInfo& shaderSetInfo );

   VkPipelineLayout findOrCreate( const CYD::PipelineLayoutInfo& pipLayoutInfo );

   VkPipeline findOrCreate( const CYD::GraphicsPipelineInfo& pipInfo, VkRenderPass renderPass );
   VkPipeline findOrCreate( const CYD::ComputePipelineInfo& pipInfo );

  private:
   const Device& m_device;

   std::unique_ptr<ShaderStash> m_shaderStash;

   std::unordered_map<CYD::ShaderSetInfo, VkDescriptorSetLayout> m_descSetLayouts;
   std::unordered_map<CYD::PipelineLayoutInfo, VkPipelineLayout> m_pipLayouts;

   std::unordered_map<CYD::GraphicsPipelineInfo, VkPipeline> m_graphicsPipelines;
   std::unordered_map<CYD::ComputePipelineInfo, VkPipeline> m_computePipelines;
};
}
