#include <Graphics/PipelineInfos.h>

#include <Common/Assert.h>

namespace CYD
{
bool PushConstantRange::operator==( const PushConstantRange& other ) const
{
   return stages == other.stages && offset == other.offset && size == other.size;
}

bool ShaderBindingInfo::operator==( const ShaderBindingInfo& other ) const
{
   return type == other.type && stages == other.stages && binding == other.binding;
}

bool ShaderSetInfo::operator==( const ShaderSetInfo& other ) const
{
   return shaderBindings == other.shaderBindings;
}

bool PipelineLayoutInfo::operator==( const PipelineLayoutInfo& other ) const
{
   return ranges == other.ranges && shaderSets == other.shaderSets;
}

bool SamplerInfo::operator==( const SamplerInfo& other ) const
{
   return useAnisotropy == other.useAnisotropy && maxAnisotropy == other.maxAnisotropy &&
          magFilter == other.magFilter && minFilter == other.minFilter &&
          addressMode == other.addressMode;
}

PipelineInfo::PipelineInfo()  = default;
PipelineInfo::~PipelineInfo() = default;

void PipelineLayoutInfo::addBinding(
    ShaderResourceType type,
    PipelineStageFlag stages,
    uint8_t binding,
    uint8_t set,
    const std::string_view name )
{
   auto setIt = std::find_if(
       shaderSets.begin(),
       shaderSets.end(),
       [set]( const ShaderSetInfo& other ) { return set == other.set; } );

   if( setIt == shaderSets.end() )
   {
      // Set does not exist, create it
      shaderSets.resize( set + 1 );
      setIt = shaderSets.begin() + set;
   }

   setIt->set = set;

   std::vector<ShaderBindingInfo>& shaderBindings = setIt->shaderBindings;

   auto bindingIt = std::find_if(
       shaderBindings.begin(),
       shaderBindings.end(),
       [binding]( const ShaderBindingInfo& other ) { return binding == other.binding; } );

   if( bindingIt != shaderBindings.end() )
   {
      // This binding already exists, abort
      // TODO WARNING
      CYD_ASSERT_AND_RETURN( !"This binding already exists, skipping", return; );
   }

   ShaderBindingInfo& info = shaderBindings.emplace_back();
   info.binding            = binding;
   info.type               = type;
   info.stages             = stages;
   info.name               = name;
}

GraphicsPipelineInfo::GraphicsPipelineInfo() : PipelineInfo( PipelineType::GRAPHICS ) {}

// TODO Also compare shader constants
bool GraphicsPipelineInfo::operator==( const GraphicsPipelineInfo& other ) const
{
   return pipLayout == other.pipLayout && drawPrim == other.drawPrim &&
          polyMode == other.polyMode && extent == other.extent && shaders == other.shaders;
}
GraphicsPipelineInfo::~GraphicsPipelineInfo() = default;

ComputePipelineInfo::ComputePipelineInfo() : PipelineInfo( PipelineType::COMPUTE ) {}

bool ComputePipelineInfo::operator==( const ComputePipelineInfo& other ) const
{
   return pipLayout == other.pipLayout && shader == other.shader;
}
ComputePipelineInfo::~ComputePipelineInfo() = default;
}