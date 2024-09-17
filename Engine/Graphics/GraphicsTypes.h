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
using MeshIndex                             = size_t;
static constexpr MeshIndex INVALID_MESH_IDX = std::numeric_limits<MeshIndex>::max();

using MaterialIndex                                 = size_t;
static constexpr MaterialIndex INVALID_MATERIAL_IDX = std::numeric_limits<MaterialIndex>::max();

using PipelineIndex                                 = size_t;
static constexpr PipelineIndex INVALID_PIPELINE_IDX = std::numeric_limits<PipelineIndex>::max();

static constexpr uint32_t ALL_MIP_LEVELS   = 0xFFFFFFFF;
static constexpr uint32_t ALL_ARRAY_LAYERS = 0xFFFFFFFF;

// ================================================================================================
// Flags

using Flag8  = uint8_t;
using Flag16 = uint16_t;
using Flag32 = uint32_t;

enum QueueUsage
{
   UNKNOWN  = 0,
   GRAPHICS = 1 << 0,
   COMPUTE  = 1 << 1,
   TRANSFER = 1 << 2
};
using QueueUsageFlag = Flag8;

namespace BufferUsage
{
enum BufferUsage
{
   TRANSFER_SRC = 1 << 0,
   TRANSFER_DST = 1 << 1,
   UNIFORM      = 1 << 2,
   STORAGE      = 1 << 3,
   INDEX        = 1 << 4,
   VERTEX       = 1 << 5
};
}
using BufferUsageFlag = Flag8;

namespace ImageUsage
{
enum ImageUsage : Flag32
{
   TRANSFER_SRC  = 1 << 0,
   TRANSFER_DST  = 1 << 1,
   SAMPLED       = 1 << 2,
   STORAGE       = 1 << 3,
   COLOR         = 1 << 4,
   DEPTH_STENCIL = 1 << 5
};
}
using ImageUsageFlag = Flag8;

enum MemoryType
{
   DEVICE_LOCAL  = 1 << 0,
   HOST_VISIBLE  = 1 << 1,
   HOST_COHERENT = 1 << 2
};
using MemoryTypeFlag = Flag8;

namespace PipelineStage
{
enum PipelineStage : Flag32
{
   VERTEX_STAGE        = 1 << 0,
   TESS_CONTROL_STAGE  = 1 << 1,
   TESS_EVAL_STAGE     = 1 << 2,
   GEOMETRY_STAGE      = 1 << 3,
   FRAGMENT_STAGE      = 1 << 4,
   COMPUTE_STAGE       = 1 << 5,
   TRANSFER_STAGE      = 1 << 6,
   ALL_GRAPHICS_STAGES = 1 << 7,
   ALL_STAGES          = 1 << 8
};
}
using PipelineStageFlag = Flag16;

// ================================================================================================
// Types & Enums

enum class PipelineType : uint8_t
{
   GRAPHICS,
   COMPUTE
};

enum class IndexType : uint8_t
{
   UNSIGNED_INT8,
   UNSIGNED_INT16,
   UNSIGNED_INT32,
};

enum class PixelFormat : uint8_t
{
   UNKNOWN,
   BGRA8_UNORM,
   RGBA8_UNORM,
   RGBA8_SRGB,
   RGBA16F,
   RGBA32F,
   RGB32F,
   RG32F,
   R32F,
   R8_UNORM,
   R16_UNORM,
   R32_UINT,
   D32_SFLOAT
};

// TODO Remove this
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

enum class Access
{
   UNDEFINED,
   VERTEX_SHADER_READ,
   FRAGMENT_SHADER_READ,
   COMPUTE_SHADER_READ,
   ANY_SHADER_READ,
   COLOR_ATTACHMENT_READ,
   DEPTH_STENCIL_ATTACHMENT_READ,
   TRANSFER_READ,
   HOST_READ,
   VERTEX_SHADER_WRITE,
   FRAGMENT_SHADER_WRITE,
   COMPUTE_SHADER_WRITE,
   ANY_SHADER_WRITE,
   COLOR_ATTACHMENT_WRITE,
   DEPTH_STENCIL_ATTACHMENT_WRITE,
   TRANSFER_WRITE,
   HOST_WRITE,
   PRESENT,
   GENERAL
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
   PATCHES
};

enum class ImageType
{
   TEXTURE_1D,
   TEXTURE_2D,
   TEXTURE_2D_ARRAY,
   TEXTURE_3D,
   TEXTURE_CUBE,
   TEXTURE_CUBE_ARRAY
};

enum class ShaderResourceType
{
   PUSH_CONSTANT,
   UNIFORM,
   SAMPLER,
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

enum class BorderColor
{
   OPAQUE_BLACK,
   TRANSPARENT_BLACK,
   OPAQUE_WHITE,
};

enum class CompareOperator
{
   NEVER,
   LESS,
   EQUAL,
   LESS_EQUAL,
   GREATER,
   GREATER_EQUAL,
   NOT_EQUAL,
   ALWAYS
};

// ================================================================================================
// Basic structs
struct TextureDescription
{
   // Dimensions
   uint32_t width  = 0;  // 2D Dimensions are optional when
   uint32_t height = 0;  // loading from storage
   uint32_t depth  = 1;

   //  Usage
   ImageType type           = ImageType::TEXTURE_2D;  // 1D, 2D, 3D...
   PixelFormat format       = PixelFormat::RGBA32F;   // The texture's pixel format
   ImageUsageFlag usage     = 0;                      // How this image will be used
   PipelineStageFlag stages = 0;                      // Stages where this texture is accessed

   // Flags
   bool generateMipmaps : 1 = false;

   std::string_view name = "Unknown Texture Name";
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

struct SwapchainInfo
{
   uint32_t imageCount;
   Extent2D extent;
   PixelFormat format;
   ColorSpace space;
   PresentMode mode;
};

union ClearColorValue
{
   float f32[4];
   int32_t i32[4];
   uint32_t u32[4];
};

struct ClearDepthStencilValue
{
   float depth;
   uint32_t stencil;
};

struct ClearValue
{
   ClearColorValue color;
   ClearDepthStencilValue depthStencil;
};

struct SamplerInfo
{
   SamplerInfo() = default;
   SamplerInfo( Filter magFilter, Filter minFilter, AddressMode addressMode )
       : magFilter( magFilter ), minFilter( minFilter ), addressMode( addressMode )
   {
   }
   bool operator==( const SamplerInfo& other ) const;

   bool useCompare         = false;
   float maxAnisotropy     = 0.0f;
   float minLod            = 0.0f;
   float maxLod            = 0.0f;
   CompareOperator compare = CompareOperator::ALWAYS;
   Filter magFilter        = Filter::LINEAR;
   Filter minFilter        = Filter::LINEAR;
   AddressMode addressMode = AddressMode::CLAMP_TO_EDGE;
   BorderColor borderColor = BorderColor::OPAQUE_BLACK;
};

// ================================================================================================
// Helper functions
bool IsColorFormat( PixelFormat format );
uint32_t GetPixelSizeInBytes( PixelFormat format );
uint32_t GetChannelsCount( PixelFormat format );

// ================================================================================================
// Transfer structs

// GPU to GPU
struct BufferCopyInfo
{
   size_t srcOffset;
   size_t dstOffset;
   size_t size;
};

// GPU to GPU
struct TextureCopyInfo
{
   Offset2D srcOffset;
   Offset2D dstOffset;
   Extent2D extent;
};

// GPU to GPU
struct BufferToTextureInfo
{
   size_t srcOffset;
   Offset2D dstOffset;
   Extent2D dstExtent;
};

// CPU to GPU
struct UploadToBufferInfo
{
   size_t srcOffset;
   size_t size;
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
struct std::hash<CYD::SamplerInfo>
{
   size_t operator()( const CYD::SamplerInfo& samplerInfo ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, samplerInfo.maxAnisotropy );
      hashCombine( seed, samplerInfo.magFilter );
      hashCombine( seed, samplerInfo.minFilter );
      hashCombine( seed, samplerInfo.addressMode );

      return seed;
   }
};
