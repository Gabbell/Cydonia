#include <Graphics/Vulkan/PipelineCache.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/GraphicsTypes.h>
#include <Graphics/VertexLayout.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/Shader.h>
#include <Graphics/Vulkan/ShaderCache.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <array>
#include <unordered_set>
#include <vector>

namespace vk
{
PipelineCache::PipelineCache( const Device& device ) : m_device( device )
{
   m_ShaderCache = std::make_unique<ShaderCache>( m_device );
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

VkDescriptorSetLayout PipelineCache::findOrCreate( const CYD::ShaderSetInfo& shaderSetInfo )
{
   // Creating the descriptor set layout
   const auto layoutIt = m_descSetLayouts.find( shaderSetInfo );
   if( layoutIt != m_descSetLayouts.end() )
   {
      return layoutIt->second;
   }

   std::vector<VkDescriptorSetLayoutBinding> descSetLayoutBindings;
   std::vector<VkDescriptorBindingFlagsEXT> descBindingFlags;
   descSetLayoutBindings.reserve( shaderSetInfo.shaderBindings.size() );
   for( const auto& bindingInfo : shaderSetInfo.shaderBindings )
   {
      // TODO Add UBO arrays
      VkDescriptorSetLayoutBinding descSetLayoutBinding = {};
      descSetLayoutBinding.binding                      = bindingInfo.binding;
      descSetLayoutBinding.descriptorType =
          TypeConversions::cydToVkDescriptorType( bindingInfo.type );
      descSetLayoutBinding.descriptorCount = 1;  // For arrays
      descSetLayoutBinding.stageFlags = TypeConversions::cydToVkShaderStages( bindingInfo.stages );
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
   CYDASSERT( result == VK_SUCCESS && "PipelineCache: Could not create descriptor set layout" );

   return m_descSetLayouts.insert( { shaderSetInfo, descSetLayout } ).first->second;
}

VkPipelineLayout PipelineCache::findOrCreate( const CYD::PipelineLayoutInfo& pipLayoutInfo )
{
   const auto layoutIt = m_pipLayouts.find( pipLayoutInfo );
   if( layoutIt != m_pipLayouts.end() )
   {
      return layoutIt->second;
   }

   std::vector<VkPushConstantRange> vkRanges;
   vkRanges.reserve( pipLayoutInfo.ranges.size() );
   for( const auto& range : pipLayoutInfo.ranges )
   {
      VkPushConstantRange vkRange = {};
      vkRange.stageFlags          = TypeConversions::cydToVkShaderStages( range.stages );
      vkRange.offset              = static_cast<uint32_t>( range.offset );
      vkRange.size                = static_cast<uint32_t>( range.size );

      vkRanges.push_back( vkRange );
   }

   std::unordered_set<VkDescriptorSetLayout> descSetLayouts;
   descSetLayouts.reserve( pipLayoutInfo.shaderSets.size() );
   for( const auto& shaderSetLayoutPair : pipLayoutInfo.shaderSets )
   {
      descSetLayouts.insert( findOrCreate( shaderSetLayoutPair.second ) );
   }
   // Vector containing unique VkDescriptorSetLayouts
   std::vector<VkDescriptorSetLayout> descSetLayoutsVec(
       descSetLayouts.cbegin(), descSetLayouts.cend() );

   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
   pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount         = static_cast<uint32_t>( descSetLayoutsVec.size() );
   pipelineLayoutInfo.pSetLayouts            = descSetLayoutsVec.data();
   pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>( vkRanges.size() );
   pipelineLayoutInfo.pPushConstantRanges    = vkRanges.data();

   VkPipelineLayout pipLayout;
   const VkResult result =
       vkCreatePipelineLayout( m_device.getVKDevice(), &pipelineLayoutInfo, nullptr, &pipLayout );
   CYDASSERT( result == VK_SUCCESS && "PipelineCache: Could not create pipeline layout" );

   return m_pipLayouts.insert( { pipLayoutInfo, pipLayout } ).first->second;
}

VkPipeline PipelineCache::findOrCreate( const CYD::ComputePipelineInfo& pipInfo )
{
   // Attempting to find pipeline
   const auto pipIt = m_computePipelines.find( pipInfo );
   if( pipIt != m_computePipelines.end() )
   {
      return pipIt->second;
   }

   VkResult result;

   // Building shader constants
   const CYD::ShaderConstants::Entry* entry = pipInfo.constants.getEntry( pipInfo.shader );

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

   const Shader* shader = m_ShaderCache->getShader( pipInfo.shader );

   VkPipelineShaderStageCreateInfo stageInfo = {};
   stageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   stageInfo.stage                           = shaderTypeToVKShaderStage( shader->getType() );
   stageInfo.module                          = shader->getModule();
   stageInfo.pName                           = "main";
   stageInfo.pSpecializationInfo             = &specInfo;

   // Pipeline layout
   VkPipelineLayout pipLayout = findOrCreate( pipInfo.pipLayout );

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
   CYDASSERT( result == VK_SUCCESS && "PipelineCache: Could not create compute pipeline" );

   return m_computePipelines.insert( { pipInfo, pipeline } ).first->second;
}

VkPipeline PipelineCache::findOrCreate(
    const CYD::GraphicsPipelineInfo& pipInfo,
    VkRenderPass renderPass )
{
   // Attempting to find pipeline
   const auto pipIt = m_graphicsPipelines.find( pipInfo );
   if( pipIt != m_graphicsPipelines.end() )
   {
      return pipIt->second;
   }

   VkResult result;

   // Scope protection for shader info structs
   std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;
   std::vector<VkSpecializationInfo> specInfos;
   std::vector<VkSpecializationMapEntry> specMapEntries;

   shaderCreateInfos.reserve( pipInfo.shaders.size() );
   specInfos.reserve( pipInfo.shaders.size() );

   // Fetching shaders
   for( const std::string& shaderName : pipInfo.shaders )
   {
      const Shader* shader = m_ShaderCache->getShader( shaderName );

      // Building shader constants
      VkSpecializationInfo* pSpecInfo          = nullptr;
      const CYD::ShaderConstants::Entry* entry = pipInfo.constants.getEntry( shaderName );

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
   const std::vector<CYD::VertexLayout::Attribute>& attributes = pipInfo.vertLayout.getAttributes();
   std::vector<VkVertexInputAttributeDescription> vkAttributes( attributes.size() );
   std::vector<VkVertexInputBindingDescription> vkBindings;

   uint32_t vertBindingStride = 0;
   for( uint32_t i = 0; i < attributes.size(); ++i )
   {
      CYD::PixelFormat vecFormat = attributes[i].vecFormat;

      vkAttributes[i].binding  = attributes[i].binding;
      vkAttributes[i].location = attributes[i].location;
      vkAttributes[i].format   = TypeConversions::cydToVkFormat( vecFormat );
      vkAttributes[i].offset   = attributes[i].offset;

      vertBindingStride += GetPixelSizeInBytes( vecFormat );
   }

   // TODO More than one binding. For now, if we have attributes, we always have one vertex binding
   if( !vkAttributes.empty() )
   {
      // TODO Instancing
      VkVertexInputBindingDescription vertexBindingDesc = {};
      vertexBindingDesc.binding                         = 0;
      vertexBindingDesc.stride                          = vertBindingStride;
      vertexBindingDesc.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;
      vkBindings.push_back( std::move( vertexBindingDesc ) );
   }

   VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   vertexInputInfo.vertexBindingDescriptionCount   = static_cast<uint32_t>( vkBindings.size() );
   vertexInputInfo.pVertexBindingDescriptions      = vkBindings.data();
   vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>( vkAttributes.size() );
   vertexInputInfo.pVertexAttributeDescriptions    = vkAttributes.data();

   // Input assembly
   VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
   inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   inputAssembly.topology = TypeConversions::cydToVkDrawPrim( pipInfo.drawPrim );
   inputAssembly.primitiveRestartEnable = VK_FALSE;

   // Viewport and scissor
   VkViewport viewport = {};
   viewport.x          = 0.0f;
   viewport.y          = 0.0f;
   viewport.width      = static_cast<float>( pipInfo.extent.width );
   viewport.height     = static_cast<float>( pipInfo.extent.height );
   viewport.minDepth   = 0.0f;
   viewport.maxDepth   = 1.0f;

   VkRect2D scissor = {};
   scissor.offset   = { 0, 0 };
   scissor.extent   = {
       static_cast<uint32_t>( pipInfo.extent.width ),
       static_cast<uint32_t>( pipInfo.extent.height ) };

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
   rasterizer.polygonMode             = TypeConversions::cydToVkPolyMode( pipInfo.polyMode );
   rasterizer.lineWidth               = 1.0f;
   rasterizer.cullMode                = VK_CULL_MODE_NONE;
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
   colorBlendAttachment.blendEnable         = VK_TRUE;
   colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
   colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
   colorBlendAttachment.colorBlendOp        = VK_BLEND_OP_ADD;
   colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
   colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
   colorBlendAttachment.alphaBlendOp        = VK_BLEND_OP_ADD;

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
   VkPipelineLayout pipLayout = findOrCreate( pipInfo.pipLayout );

   // Dynamic state
   std::vector<VkDynamicState> dynamicStates = {
       VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

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
   pipelineInfo.pTessellationState           = nullptr;
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
   CYDASSERT( result == VK_SUCCESS && "PipelineCache: Could not create graphics pipeline" );

   return m_graphicsPipelines.insert( { pipInfo, pipeline } ).first->second;
}

PipelineCache::~PipelineCache()
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
