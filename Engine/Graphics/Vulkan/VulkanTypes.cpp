#pragma once

#include <Graphics/Vulkan/VulkanTypes.h>

namespace vk
{
bool Attachment::operator==( const Attachment& other ) const
{
   return format == other.format && loadOp == other.loadOp && storeOp == other.storeOp &&
          type == other.type && initialAccess == other.initialAccess &&
          nextAccess == other.nextAccess;
}

bool RenderPassInfo::operator==( const RenderPassInfo& other ) const
{
   if( attachments.size() != other.attachments.size() ) return false;

   for( uint32_t i = 0; i < attachments.size(); ++i )
   {
      if( !( attachments[i] == other.attachments[i] ) ) return false;
   }

   return true;
}
}