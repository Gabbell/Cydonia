#include <Core/Graphics/Vulkan/Types.h>

#include <functional>

namespace cyd
{
// ================================================================================================
// Operator overloads
bool Extent::operator==( const Extent& other ) const
{
   return width == other.width && height == other.height;
}

bool Attachment::operator==( const Attachment& other ) const
{
   return format == other.format && loadOp == other.loadOp && storeOp == other.storeOp &&
          type == other.type && usage == other.usage;
}

bool RenderPassInfo::operator==( const RenderPassInfo& other ) const
{
   bool same = true;
   same      = attachments.size() == other.attachments.size();
   if( !same ) return false;

   for( uint32_t i = 0; i < attachments.size(); ++i )
   {
      same = attachments[i] == other.attachments[i];
      if( !same ) return false;
   }

   return true;
}

bool PushConstantRange::operator==( const PushConstantRange& other ) const
{
   return stages == other.stages && offset == other.offset && size == other.size;
}

bool ShaderObjectInfo::operator==( const ShaderObjectInfo& other ) const
{
   return stages == other.stages && binding == other.binding;
}

bool DescriptorSetLayoutInfo::operator==( const DescriptorSetLayoutInfo& other ) const
{
   return shaderObjects == other.shaderObjects;
}

bool PipelineLayoutInfo::operator==( const PipelineLayoutInfo& other ) const
{
   return ranges == other.ranges;
}

bool PipelineInfo::operator==( const PipelineInfo& other ) const
{
   return pipLayout == other.pipLayout && drawPrim == other.drawPrim &&
          polyMode == other.polyMode && extent == other.extent && shaders == other.shaders;
}

bool SamplerInfo::operator==( const SamplerInfo& other ) const
{
   return useAnisotropy == other.useAnisotropy && maxAnisotropy == other.maxAnisotropy &&
          magFilter == other.magFilter && minFilter == other.minFilter &&
          addressMode == other.addressMode;
}
}
