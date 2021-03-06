#pragma once

#include <Common/Include.h>

#include <glm/glm.hpp>

#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include <map>

namespace CYD
{
// ================================================================================================
// Types & Enums

using Flag = uint32_t;

enum QueueUsage : Flag
{
   UNKNOWN  = 0,
   GRAPHICS = 1 << 0,
   COMPUTE  = 1 << 1,
   TRANSFER = 1 << 2
};
using QueueUsageFlag = Flag;

namespace BufferUsage
{
enum BufferUsage : Flag
{
   TRANSFER_SRC = 1 << 0,
   TRANSFER_DST = 1 << 1,
   UNIFORM      = 1 << 2,
   STORAGE      = 1 << 3,
   INDEX        = 1 << 4,
   VERTEX       = 1 << 5
};
}
using BufferUsageFlag = Flag;

namespace ImageUsage
{
enum ImageUsage : Flag
{
   TRANSFER_SRC  = 1 << 0,
   TRANSFER_DST  = 1 << 1,
   SAMPLED       = 1 << 2,
   STORAGE       = 1 << 3,
   COLOR         = 1 << 4,
   DEPTH_STENCIL = 1 << 5
};
}
using ImageUsageFlag = Flag;

enum MemoryType : Flag
{
   DEVICE_LOCAL  = 1 << 0,
   HOST_VISIBLE  = 1 << 1,
   HOST_COHERENT = 1 << 2
};
using MemoryTypeFlag = Flag;

namespace ShaderStage
{
enum ShaderStage : Flag
{
   VERTEX_STAGE        = 1 << 0,
   GEOMETRY_STAGE      = 1 << 1,
   FRAGMENT_STAGE      = 1 << 2,
   COMPUTE_STAGE       = 1 << 3,
   ALL_GRAPHICS_STAGES = 1 << 4,
   ALL_STAGES          = 1 << 5
};
}
using ShaderStageFlag = Flag;

enum class PipelineType
{
   GRAPHICS,
   COMPUTE
};

enum class IndexType
{
   UNSIGNED_INT8,
   UNSIGNED_INT16,
   UNSIGNED_INT32,
};

enum class PixelFormat
{
   BGRA8_UNORM,
   RGBA8_SRGB,
   RGBA16F,
   RGBA32F,
   RGB32F,
   RG32F,
   R32F,
   R8_UNORM,
   R16_UNORM,
   D32_SFLOAT
};

uint32_t GetPixelSizeInBytes( PixelFormat format );

enum class ImageLayout
{
   UNKNOWN,
   GENERAL,
   COLOR_ATTACHMENT,
   DEPTH_ATTACHMENT,
   DEPTH_STENCIL_ATTACHMENT,
   DEPTH_STENCIL_READ,
   SHADER_READ,
   TRANSFER_SRC,
   TRANSFER_DST,
   PRESENT_SRC
};

enum class ColorSpace
{
   SRGB_NONLINEAR
};

enum class PresentMode
{
   IMMEDIATE,
   FIFO,
   FIFO_RELAXED,
   MAILBOX
};

enum class LoadOp
{
   LOAD,
   CLEAR,
   DONT_CARE
};

enum class StoreOp
{
   STORE,
   DONT_CARE
};

enum class AttachmentType
{
   COLOR,
   COLOR_PRESENTATION,
   DEPTH,
   DEPTH_STENCIL
};

enum class DrawPrimitive
{
   POINTS,
   LINES,
   LINE_STRIPS,
   TRIANGLES,
   TRIANGLE_STRIPS,
};

enum class PolygonMode
{
   FILL,
   LINE,
   POINT
};

enum class ImageType
{
   TEXTURE_1D,
   TEXTURE_2D,
   TEXTURE_3D
};

enum class ShaderResourceType
{
   UNIFORM,
   STORAGE,
   COMBINED_IMAGE_SAMPLER,
   STORAGE_IMAGE,
   SAMPLED_IMAGE
};

enum class Filter
{
   NEAREST,
   LINEAR,
   CUBIC
};

enum class AddressMode
{
   REPEAT,
   MIRRORED_REPEAT,
   CLAMP_TO_EDGE,
   CLAMP_TO_BORDER,
   MIRROR_CLAMP_TO_EDGE,
};

// ================================================================================================
// Basic structs

struct TextureDescription
{
   size_t size            = 0;
   uint32_t width         = 0;
   uint32_t height        = 0;
   uint32_t layers        = 1;
   ImageType type         = ImageType::TEXTURE_2D;  // 1D, 2D, 3D...
   PixelFormat format     = PixelFormat::RGBA32F;   // The texture's pixel format
   ImageUsageFlag usage   = 0;                      // How this image will be used
   ShaderStageFlag stages = 0;                      // Stages where this texture is accessed
   std::string_view name  = "Unknown Texture Name";
};

struct Extent2D
{
   bool operator==( const Extent2D& other ) const;
   uint32_t width  = 0;
   uint32_t height = 0;
};

struct Offset2D
{
   int32_t x = 0;
   int32_t y = 0;
};

struct Rectangle
{
   Offset2D offset;
   Extent2D extent;
};

struct Viewport
{
   float offsetX  = 0.0f;
   float offsetY  = 0.0f;
   float width    = 0.0f;
   float height   = 0.0f;
   float minDepth = 0.0f;
   float maxDepth = 1.0f;
};

struct Attachment
{
   bool operator==( const Attachment& other ) const;
   PixelFormat format        = PixelFormat::BGRA8_UNORM;
   AttachmentType type       = AttachmentType::COLOR;
   LoadOp loadOp             = LoadOp::DONT_CARE;
   StoreOp storeOp           = StoreOp::DONT_CARE;
   ImageLayout initialLayout = ImageLayout::UNKNOWN;
};

struct PushConstantRange
{
   bool operator==( const PushConstantRange& other ) const;
   ShaderStageFlag stages;
   size_t offset;
   size_t size;
};

// ================================================================================================
// Pipeline Description

struct SamplerInfo
{
   bool operator==( const SamplerInfo& other ) const;
   bool useAnisotropy      = true;
   float maxAnisotropy     = 16.0f;
   Filter magFilter        = Filter::LINEAR;
   Filter minFilter        = Filter::LINEAR;
   AddressMode addressMode = AddressMode::REPEAT;
};

struct RenderTargetsInfo
{
   bool operator==( const RenderTargetsInfo& other ) const;
   std::vector<Attachment> attachments;
};

struct ShaderBindingInfo
{
   bool operator==( const ShaderBindingInfo& other ) const;
   std::string name;
   ShaderResourceType type;
   ShaderStageFlag stages;
   uint32_t binding;
};

struct ShaderSetInfo
{
   bool operator==( const ShaderSetInfo& other ) const;
   // TODO Helper function to add bindings more easily
   std::vector<ShaderBindingInfo> shaderBindings;
};

struct PipelineLayoutInfo
{
   bool operator==( const PipelineLayoutInfo& other ) const;
   std::vector<PushConstantRange> ranges;
   std::map<uint32_t, ShaderSetInfo> shaderSets;
};

struct SwapchainInfo
{
   uint32_t imageCount;
   Extent2D extent;
   PixelFormat format;
   ColorSpace space;
   PresentMode mode;
};
}

// ================================================================================================
// Hashing Functions
template <>
struct std::hash<CYD::Extent2D>
{
   size_t operator()( const CYD::Extent2D& extent ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, extent.width );
      hashCombine( seed, extent.height );
      return seed;
   }
};

template <>
struct std::hash<CYD::Attachment>
{
   size_t operator()( const CYD::Attachment& attachment ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, attachment.format );
      hashCombine( seed, attachment.loadOp );
      hashCombine( seed, attachment.storeOp );
      hashCombine( seed, attachment.type );

      return seed;
   }
};

template <>
struct std::hash<CYD::ShaderBindingInfo>
{
   size_t operator()( const CYD::ShaderBindingInfo& shaderObject ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, shaderObject.type );
      hashCombine( seed, shaderObject.binding );
      hashCombine( seed, shaderObject.stages );
      return seed;
   }
};

template <>
struct std::hash<CYD::PushConstantRange>
{
   size_t operator()( const CYD::PushConstantRange& range ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, range.stages );
      hashCombine( seed, range.offset );
      hashCombine( seed, range.size );
      return seed;
   }
};

template <>
struct std::hash<CYD::RenderTargetsInfo>
{
   size_t operator()( const CYD::RenderTargetsInfo& targetsInfo ) const noexcept
   {
      const std::vector<CYD::Attachment>& attachments = targetsInfo.attachments;

      size_t seed = 0;
      for( const auto& attachment : attachments )
      {
         hashCombine( seed, attachment );
      }

      return seed;
   }
};

template <>
struct std::hash<CYD::ShaderSetInfo>
{
   size_t operator()( const CYD::ShaderSetInfo& shaderSetInfo ) const noexcept
   {
      size_t seed = 0;
      for( const auto& ubo : shaderSetInfo.shaderBindings )
      {
         hashCombine( seed, ubo );
      }
      return seed;
   }
};

template <>
struct std::hash<CYD::PipelineLayoutInfo>
{
   size_t operator()( const CYD::PipelineLayoutInfo& pipLayoutInfo ) const noexcept
   {
      size_t seed = 0;
      for( const auto& range : pipLayoutInfo.ranges )
      {
         hashCombine( seed, range );
      }
      return seed;
   }
};

template <>
struct std::hash<CYD::SamplerInfo>
{
   size_t operator()( const CYD::SamplerInfo& samplerInfo ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, samplerInfo.useAnisotropy );
      hashCombine( seed, samplerInfo.maxAnisotropy );
      hashCombine( seed, samplerInfo.magFilter );
      hashCombine( seed, samplerInfo.minFilter );
      hashCombine( seed, samplerInfo.addressMode );

      return seed;
   }
};
