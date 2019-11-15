#include <Core/Graphics/Vulkan/PipelineStash.h>

#include <Core/Common/Vulkan.h>
#include <Core/Common/Assert.h>

#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/Shader.h>
#include <Core/Graphics/Vulkan/ShaderStash.h>
#include <Core/Graphics/Vulkan/RenderPassStash.h>

#include <array>

static constexpr char DEFAULT_VERT[] = "default_vert.spv";
static constexpr char DEFAULT_FRAG[] = "default_frag.spv";

cyd::PipelineStash::PipelineStash( const Device& device ) : _device( device )
{
   _shaderStash = std::make_unique<ShaderStash>( _device );
}

static VkShaderStageFlagBits shaderTypeToVKShaderStage( cyd::Shader::Type shaderType )
{
   switch( shaderType )
   {
      case cyd::Shader::Type::VERTEX:
         return VK_SHADER_STAGE_VERTEX_BIT;
      case cyd::Shader::Type::FRAGMENT:
         return VK_SHADER_STAGE_FRAGMENT_BIT;
      case cyd::Shader::Type::COMPUTE:
         return VK_SHADER_STAGE_COMPUTE_BIT;
      default:
         return VK_SHADER_STAGE_ALL;
   }
}

const VkDescriptorSetLayout cyd::PipelineStash::findOrCreate( const DescriptorSetLayoutInfo& info )
{
   // Creating the descriptor set layout
   const auto layoutIt = _descSetLayouts.find( info );
   if( layoutIt != _descSetLayouts.end() )
   {
      return layoutIt->second;
   }

   std::vector<VkDescriptorSetLayoutBinding> descSetLayoutBindings;
   descSetLayoutBindings.reserve( info.shaderObjects.size() );
   for( const auto& object : info.shaderObjects )
   {
      // TODO Add UBO arrays
      VkDescriptorSetLayoutBinding descSetLayoutBinding = {};
      descSetLayoutBinding.binding                      = object.binding;

      switch( object.type )
      {
         case ShaderObjectType::UNIFORM:
            descSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            break;
         case ShaderObjectType::COMBINED_IMAGE_SAMPLER:
            descSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            break;
         default:
            CYDASSERT( !"PipelineStash: Descriptor type not yet implemented" );
      }

      descSetLayoutBinding.descriptorCount    = 1;  // For arrays
      descSetLayoutBinding.stageFlags         = cydShaderStagesToVkShaderStages( object.stages );
      descSetLayoutBinding.pImmutableSamplers = nullptr;

      descSetLayoutBindings.push_back( std::move( descSetLayoutBinding ) );
   }

   VkDescriptorSetLayoutCreateInfo layoutInfo = {};
   layoutInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   layoutInfo.bindingCount = static_cast<uint32_t>( descSetLayoutBindings.size() );
   layoutInfo.pBindings    = descSetLayoutBindings.data();

   VkDescriptorSetLayout descSetLayout;
   VkResult result =
       vkCreateDescriptorSetLayout( _device.getVKDevice(), &layoutInfo, nullptr, &descSetLayout );
   CYDASSERT( result == VK_SUCCESS && "PipelineStash: Could not create descriptor set layout" );

   _descSetLayouts.insert( { info, descSetLayout } );

   return descSetLayout;
}

const VkPipelineLayout cyd::PipelineStash::findOrCreate( const PipelineLayoutInfo& info )
{
   const auto layoutIt = _pipLayouts.find( info );
   if( layoutIt != _pipLayouts.end() )
   {
      return layoutIt->second;
   }

   std::vector<VkPushConstantRange> vkRanges;
   vkRanges.reserve( info.ranges.size() );
   for( const auto& range : info.ranges )
   {
      VkPushConstantRange vkRange = {};
      vkRange.stageFlags          = cydShaderStagesToVkShaderStages( range.stages );
      vkRange.offset              = range.offset;
      vkRange.size                = range.size;

      vkRanges.push_back( std::move( vkRange ) );
   }

   // TODO This vector contains multiple copies of the same layout. Why is this necessary? The two
   // layouts will have information about the same multiple bindings. This seems redundant.
   std::vector<VkDescriptorSetLayout> descSetLayouts(
       info.descSetLayout.shaderObjects.size(), findOrCreate( info.descSetLayout ) );

   VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};

   pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
   pipelineLayoutInfo.setLayoutCount         = static_cast<uint32_t>( descSetLayouts.size() );
   pipelineLayoutInfo.pSetLayouts            = descSetLayouts.data();
   pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>( vkRanges.size() );
   pipelineLayoutInfo.pPushConstantRanges    = vkRanges.data();

   VkPipelineLayout pipLayout;
   VkResult result =
       vkCreatePipelineLayout( _device.getVKDevice(), &pipelineLayoutInfo, nullptr, &pipLayout );
   CYDASSERT( result == VK_SUCCESS && "PipelineStash: Could not create pipeline layout" );

   _pipLayouts.insert( { info, pipLayout } );

   return pipLayout;
}

const VkPipeline cyd::PipelineStash::findOrCreate( const PipelineInfo& info )
{
   // Attempting to find pipeline
   const auto pipIt = _pipelines.find( info );
   if( pipIt != _pipelines.end() )
   {
      return pipIt->second;
   }

   VkResult result;

   // Fetching shaders
   std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;
   shaderCreateInfos.reserve( info.shaders.size() );
   for( const std::string& shaderName : info.shaders )
   {
      const Shader* shader = _shaderStash->getShader( shaderName );

      VkPipelineShaderStageCreateInfo stageInfo = {};
      stageInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      stageInfo.stage               = shaderTypeToVKShaderStage( shader->getType() );
      stageInfo.module              = shader->getModule();
      stageInfo.pName               = "main";
      stageInfo.pSpecializationInfo = nullptr;  // TODO SPEC CONSTS

      shaderCreateInfos.push_back( std::move( stageInfo ) );
   }

   // Fetching render pass
   const VkRenderPass renderPass = _device.getRenderPassStash().findOrCreate( info.renderPass );

   // Vertex input description
   // TODO Instancing
   VkVertexInputBindingDescription vertexBindingDesc = {};
   vertexBindingDesc.binding                         = 0;
   vertexBindingDesc.stride                          = sizeof( Vertex );
   vertexBindingDesc.inputRate                       = VK_VERTEX_INPUT_RATE_VERTEX;

   // Vertex attributes
   std::array<VkVertexInputAttributeDescription, 3> attributeDescs = {};

   // Position
   attributeDescs[0].binding  = 0;
   attributeDescs[0].location = 0;
   attributeDescs[0].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
   attributeDescs[0].offset   = offsetof( Vertex, pos );

   // Color
   attributeDescs[1].binding  = 0;
   attributeDescs[1].location = 1;
   attributeDescs[1].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
   attributeDescs[1].offset   = offsetof( Vertex, col );

   // Texture Coordinates
   attributeDescs[2].binding  = 0;
   attributeDescs[2].location = 2;
   attributeDescs[2].format   = VK_FORMAT_R32G32B32A32_SFLOAT;
   attributeDescs[2].offset   = offsetof( Vertex, uv );

   VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
   vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   vertexInputInfo.vertexBindingDescriptionCount   = 1;
   vertexInputInfo.pVertexBindingDescriptions      = &vertexBindingDesc;
   vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>( attributeDescs.size() );
   vertexInputInfo.pVertexAttributeDescriptions    = attributeDescs.data();

   // Input assembly
   VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
   inputAssembly.sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
   inputAssembly.topology = cydDrawPrimToVkDrawPrim( info.drawPrim );
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
   scissor.extent   = { info.extent.width, info.extent.height };

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
   rasterizer.polygonMode             = cydPolyModeToVkPolyMode( info.polyMode );
   rasterizer.lineWidth               = 1.0f;
   rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
   rasterizer.frontFace               = VK_FRONT_FACE_CLOCKWISE;
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
   const VkPipelineLayout pipLayout = findOrCreate( info.pipLayout );

   // Dynamic state
   std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT };

   VkPipelineDynamicStateCreateInfo dynamicCreateInfo = {};
   dynamicCreateInfo.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
   dynamicCreateInfo.dynamicStateCount = static_cast<uint32_t>( dynamicStates.size() );
   dynamicCreateInfo.pDynamicStates    = dynamicStates.data();

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
   pipelineInfo.layout                       = pipLayout;
   pipelineInfo.renderPass                   = renderPass;
   pipelineInfo.subpass                      = 0;
   pipelineInfo.basePipelineHandle           = VK_NULL_HANDLE;

   VkPipeline pipeline;
   result = vkCreateGraphicsPipelines(
       _device.getVKDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline );
   CYDASSERT( result == VK_SUCCESS && "PipelineStash: Could not create default pipeline" );

   _pipelines.insert( { info, pipeline } );

   return pipeline;
}

cyd::PipelineStash::~PipelineStash()
{
   for( const auto& pipeline : _pipelines )
   {
      vkDestroyPipeline( _device.getVKDevice(), pipeline.second, nullptr );
   }
   for( const auto& pipLayout : _pipLayouts )
   {
      vkDestroyPipelineLayout( _device.getVKDevice(), pipLayout.second, nullptr );
   }
   for( const auto& descSetLayout : _descSetLayouts )
   {
      vkDestroyDescriptorSetLayout( _device.getVKDevice(), descSetLayout.second, nullptr );
   }
}
