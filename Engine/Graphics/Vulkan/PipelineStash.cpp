#include <Graphics/Vulkan/PipelineStash.h>

#include <Common/Vulkan.h>
#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/Shader.h>
#include <Graphics/Vulkan/ShaderStash.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <array>
#include <unordered_set>
#include <vector>

namespace vk
{
PipelineStash::PipelineStash( const Device& device ) : m_device( device )
{
   m_shaderStash = std::make_unique<ShaderStash>( m_device );
}

static VkShaderStageFlagBits shaderTypeToVKShaderStage( Shader::Type shaderType )
{
   switch( shaderType )
   {
      case Shader::Type::VERTEX:
         return VK_SHADER_STAGE_VERTEX_BIT;
      case Shader::Type::FRAGMENT:
         return VK_SHADER_STAGE_FRAGMENT_BIT;
      case Shader::Type::COMPUTE:
         return VK_SHADER_STAGE_COMPUTE_BIT;
      default:
         return VK_SHADER_STAGE_ALL;
   }
}

const VkDescriptorSetLayout PipelineStash::findOrCreate( const CYD::DescriptorSetLayoutInfo& info )
{
   // Creating the descriptor set layout
   const auto layoutIt = m_descSetLayouts.find( info );
   if( layoutIt != m_descSetLayouts.end() )
   {
      return layoutIt->second;
   }

   std::vector<VkDescriptorSetLayoutBinding> descSetLayoutBindings;
   std::vector<VkDescriptorBindingFlagsEXT> descBindingFlags;
   descSetLayoutBindings.reserve( info.shaderResources.size() );
   for( const auto& object : info.shaderResources )
   {
      // TODO Add UBO arrays
      VkDescriptorSetLayoutBinding descSetLayoutBinding = {};
      descSetLayoutBinding.binding                      = object.binding;
      descSetLayoutBinding.descriptorType  = TypeConversions::cydToVkDescriptorType( object.type );
      descSetLayoutBinding.descriptorCount = 1;  // For arrays
      descSetLayoutBinding.stageFlags      = TypeConversions::cydToVkShaderStages( object.stages );
      descSetLayoutBinding.pImmutableSamplers = nullptr;

      descSetLayoutBindings.push_back( descSetLayoutBinding );
   }

   VkDescriptorSetLayoutCreateInfo layoutInfo = {};
   layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast<uint32_t>( descSetLayoutBindings.size() );
   layoutInfo.pBindings    = descSetLayoutBindings.data();

   VkDescriptorSetLayout descSetLayout;
   VkResult result =
       vkCreateDescriptorSetLayout( m_device.getVKDevice(), &layoutInfo, nullptr, &descSetLayout );
   CYDASSERT( result == VK_SUCCESS && "PipelineStash: Could not create descriptor set layout" );

   return m_descSetLayouts.insert( { info, descSetLayout } ).first->second;
}

const VkPipelineLayout PipelineStash::findOrCreate( const CYD::PipelineLayoutInfo& info )
{
   const auto layoutIt = m_pipLayouts.find( info );
   if( layoutIt != m_pipLayouts.end() )
   {
      return layoutIt->second;
   }

   std::vector<VkPushConstantRange> vkRanges;
   vkRanges.reserve( info.ranges.size() );
   for( const auto& range : info.ranges )
   {
      VkPushConstantRange vkRange = {};
      vkRange.stageFlags          = TypeConversions::cydToVkShaderStages( range.stages );
      vkRange.offset              = static_cast<uint32_t>( range.offset );
      vkRange.size                = static_cast<uint32_t>( range.size );

      vkRanges.push_back( vkRange );
   }

   std::vector<VkDescriptorSetLayout> descSetLayouts;
   descSetLayouts.reserve( info.descSets.size() );
   for( const auto& descSetLayout : info.descSets )
   {
      descSetLayouts.push_back( findOrCreate( descSetLayout ) );
   }
   // Vector containing unique VkDescriptorSetLayouts
   std::vector<VkDescriptorSetLayout> descSetLayoutsVec(
       descSetLayouts.begin(), descSetLayouts.end() );

   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
   pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount         = static_cast<uint32_t>( descSetLayoutsVec.size() );
   pipelineLayoutInfo.pSetLayouts            = descSetLayoutsVec.data();
   pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>( vkRanges.size() );
   pipelineLayoutInfo.pPushConstantRanges    = vkRanges.data();

   VkPipelineLayout pipLayout;
   const VkResult result =
       vkCreatePipelineLayout( m_device.getVKDevice(), &pipelineLayoutInfo, nullptr, &pipLayout );
   CYDASSERT( result == VK_SUCCESS && "PipelineStash: Could not create pipeline layout" );

   return m_pipLayouts.insert( { info, pipLayout } ).first->second;
}

const VkPipeline PipelineStash::findOrCreate( const CYD::ComputePipelineInfo& info )
{
   // Attempting to find pipeline
   const auto pipIt = m_computePipelines.find( info );
   if( pipIt != m_computePipelines.end() )
   {
      return pipIt->second;
   }

   VkResult result;

   // Building shader constants
   const CYD::ShaderConstants::Entry* entry = info.constants.getEntry( info.shader );

   VkSpecializationInfo specInfo = {};
   std::vector<VkSpecializationMapEntry> specMapEntries;

   if( entry )
   {
      // This shader has at least one constant
      const CYD::ShaderConstants::InfoMap& constantInfos = entry->getConstantInfos();

      size_t prevSize = specMapEntries.size();

      for( const auto& constantInfo : constantInfos )
      {
         VkSpecializationMapEntry specMapEntry = {};
         specMapEntry.constantID               = constantInfo.first;
         specMapEntry.offset                   = constantInfo.second.offset;
         specMapEntry.size                     = constantInfo.second.size;
         specMapEntries.push_back( specMapEntry );
      }

      specInfo.mapEntryCount = static_cast<uint32_t>( specMapEntries.size() );
      specInfo.pMapEntries   = &specMapEntries[prevSize];
      specInfo.dataSize      = entry->getDataSize();
      specInfo.pData         = entry->getData();
   }

   const Shader* shader = m_shaderStash->getShader( info.shader );

   VkPipelineShaderStageCreateInfo stageInfo = {};
   stageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   stageInfo.stage                           = shaderTypeToVKShaderStage( shader->getType() );
   stageInfo.module                          = shader->getModule();
   stageInfo.pName                           = "main";
   stageInfo.pSpecializationInfo             = &specInfo;

   // Pipeline layout
   VkPipelineLayout pipLayout = findOrCreate( info.pipLayout );

   VkComputePipelineCreateInfo pipelineInfo = {};
   pipelineInfo.sType                       = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
   pipelineInfo.pNext                       = nullptr;
   pipelineInfo.flags                       = 0;
   pipelineInfo.stage                       = stageInfo;
   pipelineInfo.layout                      = pipLayout;
   pipelineInfo.basePipelineHandle          = nullptr;
   pipelineInfo.basePipelineIndex           = -1;

   VkPipeline pipeline;
   result = vkCreateComputePipelines(
       m_device.getVKDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline );
   CYDASSERT( result == VK_SUCCESS && "PipelineStash: Could not create compute pipeline" );

   return m_computePipelines.insert( { info, pipeline } ).first->second;
}

const VkPipeline PipelineStash::findOrCreate(
    const CYD::GraphicsPipelineInfo& info,
    VkRenderPass renderPass )
{
   // Attempting to find pipeline
   const auto pipIt = m_graphicsPipelines.find( info );
   if( pipIt != m_graphicsPipelines.end() )
   {
      return pipIt->second;
   }

   VkResult result;

   // Scope protection for shader info structs
   std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;
   std::vector<VkSpecializationInfo> specInfos;
   std::vector<VkSpecializationMapEntry> specMapEntries;

   shaderCreateInfos.reserve( info.shaders.size() );
   specInfos.reserve( info.shaders.size() );

   // Fetching shaders
   for( const std::string& shaderName : info.shaders )
   {
      const Shader* shader = m_shaderStash->getShader( shaderName );

      // Building shader constants
      VkSpecializationInfo* pSpecInfo          = nullptr;
      const CYD::ShaderConstants::Entry* entry = info.constants.getEntry( shaderName );

      if( entry )
      {
         // This shader has at least one constant
         const CYD::ShaderConstants::InfoMap& constantInfos = entry->getConstantInfos();

         size_t prevSize = specMapEntries.size();

         for( const auto& constantInfo : constantInfos )
         {
            VkSpecializationMapEntry specMapEntry = {};
            specMapEntry.constantID               = constantInfo.first;
            specMapEntry.offset                   = constantInfo.second.offset;
            specMapEntry.size                     = constantInfo.second.size;
            specMapEntries.push_back( specMapEntry );
         }

         VkSpecializationInfo specInfo = {};
         specInfo.mapEntryCount        = static_cast<uint32_t>( specMapEntries.size() );
         specInfo.pMapEntries          = &specMapEntries[prevSize];
         specInfo.dataSize             = entry->getDataSize();
         specInfo.pData                = entry->getData();

         specInfos.push_back( specInfo );

         pSpecInfo = &specInfos.back();
      }

      // Building shader stage
      VkPipelineShaderStageCreateInfo stageInfo = {};
      stageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stageInfo.stage               = shaderTypeToVKShaderStage( shader->getType() );
      stageInfo.module              = shader->getModule();
      stageInfo.pName               = "main";
      stageInfo.pSpecializationInfo = pSpecInfo;

      shaderCreateInfos.push_back( stageInfo );
   }

   // Vertex input description
   // TODO Instancing
   VkVertexInputBindingDescription vertexBindingDesc = {};
   vertexBindingDesc.binding                         = 0;
   vertexBindingDesc.stride                          = sizeof( CYD::Vertex );
   vertexBindingDesc.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

   // Vertex attributes
   std::array<VkVertexInputAttributeDescription, 4> attributeDescs = {};

   // Position
   attributeDescs[0].binding  = 0;
   attributeDescs[0].location = 0;
   attributeDescs[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescs[0].offset   = offsetof( CYD::Vertex, pos );

   // Color
   attributeDescs[1].binding  = 0;
   attributeDescs[1].location = 1;
   attributeDescs[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
   attributeDescs[1].offset   = offsetof( CYD::Vertex, col );

   // Texture Coordinates
   attributeDescs[3].binding  = 0;
   attributeDescs[3].location = 2;
   attributeDescs[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescs[3].offset   = offsetof( CYD::Vertex, uv );

   // Normals
   attributeDescs[2].binding  = 0;
   attributeDescs[2].location = 3;
   attributeDescs[2].format   = VK_FORMAT_R32G32B32_SFLOAT;
   attributeDescs[2].offset   = offsetof( CYD::Vertex, normal );

   VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   vertexInputInfo.vertexBindingDescriptionCount   = 1;
   vertexInputInfo.pVertexBindingDescriptions      = &vertexBindingDesc;
   vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>( attributeDescs.size() );
   vertexInputInfo.pVertexAttributeDescriptions    = attributeDescs.data();

   // Input assembly
   VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
   inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   inputAssembly.topology = TypeConversions::cydToVkDrawPrim( info.drawPrim );
   inputAssembly.primitiveRestartEnable = VK_FALSE;

   // Viewport and scissor
   VkViewport viewport = {};
   viewport.x          = 0.0f;
   viewport.y          = 0.0f;
   viewport.width      = static_cast<float>( info.extent.width );
   viewport.height     = static_cast<float>( info.extent.height );
   viewport.minDepth   = 0.0f;
   viewport.maxDepth   = 1.0f;

   VkRect2D scissor = {};
   scissor.offset   = { 0, 0 };
   scissor.extent   = {
       static_cast<uint32_t>( info.extent.width ), static_cast<uint32_t>( info.extent.height ) };

   VkPipelineViewportStateCreateInfo viewportState = {};
   viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
   viewportState.viewportCount = 1;
   viewportState.pViewports    = &viewport;
   viewportState.scissorCount  = 1;
   viewportState.pScissors     = &scissor;

   // Rasterizer
   VkPipelineRasterizationStateCreateInfo rasterizer = {};
   rasterizer.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
   rasterizer.depthClampEnable        = VK_FALSE;
   rasterizer.rasterizerDiscardEnable = VK_FALSE;
   rasterizer.polygonMode             = TypeConversions::cydToVkPolyMode( info.polyMode );
   rasterizer.lineWidth               = 1.0f;
   rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
   rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
   rasterizer.depthBiasEnable         = VK_FALSE;

   // Multisampling
   VkPipelineMultisampleStateCreateInfo multisampling = {};
   multisampling.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
   multisampling.sampleShadingEnable  = VK_FALSE;
   multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

   // Color blending
   VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
   colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                         VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
   colorBlendAttachment.blendEnable = VK_FALSE;

   VkPipelineColorBlendStateCreateInfo colorBlending = {};
   colorBlending.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
   colorBlending.logicOpEnable     = VK_FALSE;
   colorBlending.logicOp           = VK_LOGIC_OP_COPY;
   colorBlending.attachmentCount   = 1;
   colorBlending.pAttachments      = &colorBlendAttachment;
   colorBlending.blendConstants[0] = 0.0f;
   colorBlending.blendConstants[1] = 0.0f;
   colorBlending.blendConstants[2] = 0.0f;
   colorBlending.blendConstants[3] = 0.0f;

   // Pipeline layout
   VkPipelineLayout pipLayout = findOrCreate( info.pipLayout );

   // Dynamic state
   std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT };

   VkPipelineDynamicStateCreateInfo dynamicCreateInfo = {};
   dynamicCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   dynamicCreateInfo.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
   dynamicCreateInfo.pDynamicStates    = dynamicStates.data();

   // Depth stencil state
   // TODO Maybe not create a depth state when we don't have any depth attachment? Probably has
   // little to no effect on performance though
   VkPipelineDepthStencilStateCreateInfo depthStencil = {};
   depthStencil.sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
   depthStencil.depthTestEnable       = VK_TRUE;
   depthStencil.depthWriteEnable      = VK_TRUE;
   depthStencil.depthCompareOp        = VK_COMPARE_OP_LESS;
   depthStencil.depthBoundsTestEnable = VK_FALSE;
   depthStencil.stencilTestEnable     = VK_FALSE;
   depthStencil.minDepthBounds        = 0.0f;
   depthStencil.maxDepthBounds        = 1.0f;
   depthStencil.front                 = {};
   depthStencil.back                  = {};

   // Pipeline
   VkGraphicsPipelineCreateInfo pipelineInfo = {};
   pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineInfo.stageCount                   = static_cast<uint32_t>( shaderCreateInfos.size() );
   pipelineInfo.pStages                      = shaderCreateInfos.data();
   pipelineInfo.pVertexInputState            = &vertexInputInfo;
   pipelineInfo.pInputAssemblyState          = &inputAssembly;
   pipelineInfo.pViewportState               = &viewportState;
   pipelineInfo.pRasterizationState          = &rasterizer;
   pipelineInfo.pMultisampleState            = &multisampling;
   pipelineInfo.pColorBlendState             = &colorBlending;
   pipelineInfo.pDynamicState                = &dynamicCreateInfo;
   pipelineInfo.pDepthStencilState           = &depthStencil;
   pipelineInfo.layout                       = pipLayout;
   pipelineInfo.renderPass                   = renderPass;
   pipelineInfo.subpass                      = 0;
   pipelineInfo.basePipelineHandle           = nullptr;

   VkPipeline pipeline;
   result = vkCreateGraphicsPipelines(
       m_device.getVKDevice(), nullptr, 1, &pipelineInfo, nullptr, &pipeline );
   CYDASSERT( result == VK_SUCCESS && "PipelineStash: Could not create graphics pipeline" );

   return m_graphicsPipelines.insert( { info, pipeline } ).first->second;
}

PipelineStash::~PipelineStash()
{
   for( const auto& pipeline : m_graphicsPipelines )
   {
      vkDestroyPipeline( m_device.getVKDevice(), pipeline.second, nullptr );
   }
   for( const auto& pipeline : m_computePipelines )
   {
      vkDestroyPipeline( m_device.getVKDevice(), pipeline.second, nullptr );
   }
   for( const auto& pipLayout : m_pipLayouts )
   {
      vkDestroyPipelineLayout( m_device.getVKDevice(), pipLayout.second, nullptr );
   }
   for( const auto& descSetLayout : m_descSetLayouts )
   {
      vkDestroyDescriptorSetLayout( m_device.getVKDevice(), descSetLayout.second, nullptr );
   }
}
}
