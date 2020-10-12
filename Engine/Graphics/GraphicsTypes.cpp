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

bool ShaderResourceInfo::operator==( const ShaderResourceInfo& other ) const
{
   return type == other.type && stages == other.stages && binding == other.binding &&
          set == other.set;
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

bool DescriptorSetLayoutInfo::operator==( const DescriptorSetLayoutInfo& other ) const
{
   return shaderResources == other.shaderResources;
}

bool PipelineLayoutInfo::operator==( const PipelineLayoutInfo& other ) const
{
   return ranges == other.ranges && descSets == other.descSets;
}

bool SamplerInfo::operator==( const SamplerInfo& other ) const
{
   return useAnisotropy == other.useAnisotropy && maxAnisotropy == other.maxAnisotropy &&
          magFilter == other.magFilter && minFilter == other.minFilter &&
          addressMode == other.addressMode;
}
}
