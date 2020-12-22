#include <Graphics/GraphicsTypes.h>

namespace CYD
{
bool Vertex::operator==( const Vertex& other ) const
{
   return pos == other.pos && col == other.col && uv == other.uv;
}

bool Extent2D::operator==( const Extent2D& other ) const
{
   return width == other.width && height == other.height;
}

bool Attachment::operator==( const Attachment& other ) const
{
   return format == other.format && loadOp == other.loadOp && storeOp == other.storeOp &&
          type == other.type;
}

bool PushConstantRange::operator==( const PushConstantRange& other ) const
{
   return stages == other.stages && offset == other.offset && size == other.size;
}

bool ShaderBindingInfo::operator==( const ShaderBindingInfo& other ) const
{
   return type == other.type && stages == other.stages && binding == other.binding;
}

bool RenderTargetsInfo::operator==( const RenderTargetsInfo& other ) const
{
   if( attachments.size() != other.attachments.size() ) return false;

   for( uint32_t i = 0; i < attachments.size(); ++i )
   {
      if( !( attachments[i] == other.attachments[i] ) ) return false;
   }

   return true;
}

bool ShaderSetLayoutInfo::operator==( const ShaderSetLayoutInfo& other ) const
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
}
