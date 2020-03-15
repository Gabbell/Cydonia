#pragma once

#include <Graphics/ShaderConstants.h>

#include <Common/Include.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace cyd
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

enum ShaderStage : Flag
{
   VERTEX_STAGE        = 1 << 0,
   GEOMETRY_STAGE      = 1 << 1,
   FRAGMENT_STAGE      = 1 << 2,
   COMPUTE_STAGE       = 1 << 3,
   ALL_GRAPHICS_STAGES = 1 << 4,
   ALL_STAGES          = 1 << 5
};
using ShaderStageFlag = Flag;

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
   D32_SFLOAT,
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
   COMBINED_IMAGE_SAMPLER
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
   size_t size;
   uint32_t width;
   uint32_t height;
   ImageType type;
   PixelFormat format;
   ImageUsageFlag usage;
};

struct Extent
{
   bool operator==( const Extent& other ) const;
   uint32_t width  = 0;
   uint32_t height = 0;
};

struct Rectangle
{
   float offsetX;
   float offsetY;
   float width;
   float height;
};

struct Vertex
{
   // Keep in sync with the VkVertexInputAttributeDescription in PipelineStash
   bool operator==( const Vertex& other ) const;
   glm::vec3 pos;
   glm::vec4 col;
   glm::vec3 uv;
   glm::vec3 normal;
};

struct ShaderResourceInfo
{
   bool operator==( const ShaderResourceInfo& other ) const;
   uint32_t set;
   uint32_t binding;
   ShaderStageFlag stages;
   ShaderResourceType type;
};

struct Attachment
{
   bool operator==( const Attachment& other ) const;
   PixelFormat format;
   LoadOp loadOp;
   StoreOp storeOp;
   AttachmentType type;
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
   Filter magFilter        = Filter::NEAREST;
   Filter minFilter        = Filter::NEAREST;
   AddressMode addressMode = AddressMode::REPEAT;
};

struct RenderPassInfo
{
   bool operator==( const RenderPassInfo& other ) const;
   std::vector<Attachment> attachments;
};

struct DescriptorSetLayoutInfo
{
   bool operator==( const DescriptorSetLayoutInfo& other ) const;
   std::vector<ShaderResourceInfo> shaderResources;
};

struct PipelineLayoutInfo
{
   bool operator==( const PipelineLayoutInfo& other ) const;
   std::vector<PushConstantRange> ranges;
   std::vector<DescriptorSetLayoutInfo> descSets;
};

struct PipelineInfo
{
   bool operator==( const PipelineInfo& other ) const;
   std::vector<std::string> shaders;
   ShaderConstants constants;
   PipelineLayoutInfo pipLayout;
   DrawPrimitive drawPrim;
   PolygonMode polyMode;
   Extent extent;
};

struct SwapchainInfo
{
   Extent extent;
   PixelFormat format;
   ColorSpace space;
   PresentMode mode;
};
}

// ================================================================================================
// Hashing Functions

template <>
struct std::hash<cyd::Vertex>
{
   size_t operator()( const cyd::Vertex& vertex ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, vertex.pos.x );
      hashCombine( seed, vertex.pos.y );
      hashCombine( seed, vertex.pos.z );
      hashCombine( seed, vertex.col.r );
      hashCombine( seed, vertex.col.g );
      hashCombine( seed, vertex.col.b );
      hashCombine( seed, vertex.col.a );
      hashCombine( seed, vertex.uv.x );
      hashCombine( seed, vertex.uv.y );

      return seed;
   }
};

template <>
struct std::hash<cyd::Extent>
{
   size_t operator()( const cyd::Extent& extent ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, extent.width );
      hashCombine( seed, extent.height );
      return seed;
   }
};

template <>
struct std::hash<cyd::Attachment>
{
   size_t operator()( const cyd::Attachment& attachment ) const noexcept
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
struct std::hash<cyd::ShaderResourceInfo>
{
   size_t operator()( const cyd::ShaderResourceInfo& shaderObject ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, shaderObject.type );
      hashCombine( seed, shaderObject.binding );
      hashCombine( seed, shaderObject.stages );
      return seed;
   }
};

template <>
struct std::hash<cyd::PushConstantRange>
{
   size_t operator()( const cyd::PushConstantRange& range ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, range.stages );
      hashCombine( seed, range.offset );
      hashCombine( seed, range.size );
      return seed;
   }
};

template <>
struct std::hash<cyd::RenderPassInfo>
{
   size_t operator()( const cyd::RenderPassInfo& renderPass ) const noexcept
   {
      const std::vector<cyd::Attachment>& attachments = renderPass.attachments;

      size_t seed = 0;
      for( const auto& attachment : attachments )
      {
         hashCombine( seed, attachment );
      }
      return seed;
   }
};

template <>
struct std::hash<cyd::DescriptorSetLayoutInfo>
{
   size_t operator()( const cyd::DescriptorSetLayoutInfo& descSetLayoutInfo ) const noexcept
   {
      size_t seed = 0;
      for( const auto& ubo : descSetLayoutInfo.shaderResources )
      {
         hashCombine( seed, ubo );
      }
      return seed;
   }
};

template <>
struct std::hash<cyd::PipelineLayoutInfo>
{
   size_t operator()( const cyd::PipelineLayoutInfo& pipLayoutInfo ) const noexcept
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
struct std::hash<cyd::PipelineInfo>
{
   size_t operator()( const cyd::PipelineInfo& pipInfo ) const noexcept
   {
      const std::vector<std::string>& shaders = pipInfo.shaders;

      size_t seed = 0;
      hashCombine( seed, pipInfo.pipLayout );
      hashCombine( seed, pipInfo.drawPrim );
      hashCombine( seed, pipInfo.polyMode );
      hashCombine( seed, pipInfo.extent );
      for( const auto& shader : shaders )
      {
         hashCombine( seed, shader );
      }

      return seed;
   }
};

template <>
struct std::hash<cyd::SamplerInfo>
{
   size_t operator()( const cyd::SamplerInfo& samplerInfo ) const noexcept
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
