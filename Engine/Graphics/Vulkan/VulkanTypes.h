#pragma once

#include <Graphics/GraphicsTypes.h>

namespace vk
{
struct Attachment
{
   bool operator==( const Attachment& other ) const;
   CYD::PixelFormat format   = CYD::PixelFormat::BGRA8_UNORM;
   CYD::AttachmentType type  = CYD::AttachmentType::COLOR;
   CYD::LoadOp loadOp        = CYD::LoadOp::DONT_CARE;
   CYD::StoreOp storeOp      = CYD::StoreOp::DONT_CARE;
   CYD::ClearValue clear     = {};
   CYD::Access initialAccess = CYD::Access::UNDEFINED;
   CYD::Access nextAccess    = CYD::Access::GENERAL;
};

struct RenderPassInfo
{
   // TODO CLEAR VALUE IS NOT COMPARED HERE, SHOULD PROBABLY BE MOVED OUT OF RENDER PASS INFO
   bool operator==( const RenderPassInfo& other ) const;
   std::vector<Attachment> attachments;
};
};

// Hashing functions
template <>
struct std::hash<vk::Attachment>
{
   size_t operator()( const vk::Attachment& attachment ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, attachment.format );
      hashCombine( seed, attachment.type );
      hashCombine( seed, attachment.loadOp );
      hashCombine( seed, attachment.storeOp );
      hashCombine( seed, attachment.initialAccess );
      hashCombine( seed, attachment.nextAccess );

      return seed;
   }
};

template <>
struct std::hash<vk::RenderPassInfo>
{
   size_t operator()( const vk::RenderPassInfo& info ) const noexcept
   {
      const std::vector<vk::Attachment>& attachments = info.attachments;

      size_t seed = 0;
      for( const auto& attachment : attachments )
      {
         hashCombine( seed, attachment );
      }

      return seed;
   }
};
