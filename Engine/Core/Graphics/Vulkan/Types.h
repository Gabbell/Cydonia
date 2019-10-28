#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <string>
#include <vector>

// ================================================================================================
// Forwards
enum VkFormat;
enum VkColorSpaceKHR;
enum VkAttachmentLoadOp;
enum VkAttachmentStoreOp;
enum VkPrimitiveTopology;
enum VkPolygonMode;

// ================================================================================================
// Hashing utility
template <class T>
void hash_combine( size_t& seed, const T& obj )
{
   seed ^= std::hash<T>()( obj ) + 0x9e3779b9 + ( seed << 6 ) + ( seed >> 2 );
}

namespace cyd
{
// ================================================================================================
// Enums
using Flag = uint32_t;

enum QueueUsage : Flag
{
   GRAPHICS = 1 << 0,
   COMPUTE  = 1 << 1,
   TRANSFER = 1 << 2
};
using QueueUsageFlag = Flag;

enum BufferUsage : Flag
{
   TRANSFER_SRC = 1 << 0,
   TRANSFER_DST = 1 << 1,
   UNIFORM      = 1 << 2,
   STORAGE      = 1 << 3,
   INDEX        = 1 << 4,
   VERTEX       = 1 << 5
};
using BufferUsageFlag = Flag;

enum MemoryType : Flag
{
   DEVICE_LOCAL  = 1 << 0,
   HOST_VISIBLE  = 1 << 1,
   HOST_COHERENT = 1 << 2
};
using MemoryTypeFlag = Flag;

enum class ShaderStage
{
   VERTEX,
   GEOMETRY,
   FRAGMENT,
   COMPUTE,
   ALL_GRAPHICS,
   ALL
};

enum class PixelFormat
{
   BGRA8_UNORM
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
   DEPTH,
   DEPTH_STENCIL
};

enum class AttachmentUsage
{
   UNKNOWN,
   COLOR,
   PRESENTATION,
   TRANSFER_SRC,
   TRANSFER_DST,
   SHADER_READ
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

// ================================================================================================
// Basic structs
struct Vertex
{
   glm::vec4 pos;
   glm::vec4 col;
};

struct Extent
{
   bool operator==( const Extent& other ) const;
   uint32_t width;
   uint32_t height;
};

struct Attachment
{
   bool operator==( const Attachment& other ) const;
   PixelFormat format;
   LoadOp loadOp;
   StoreOp storeOp;
   AttachmentType type;
   AttachmentUsage usage;
};

// ================================================================================================
// Create infos
struct RenderPassInfo
{
   bool operator==( const RenderPassInfo& other ) const;
   std::vector<Attachment> attachments;
};

struct SwapchainInfo
{
   Extent extent;
   PixelFormat format;
   ColorSpace space;
   PresentMode mode;
};

struct PipelineLayoutInfo
{
   bool operator==( const PipelineLayoutInfo& other ) const;
   uint32_t dummy;
};

struct PipelineInfo
{
   bool operator==( const PipelineInfo& other ) const;
   RenderPassInfo renderPass;
   PipelineLayoutInfo pipLayout;
   DrawPrimitive drawPrim;
   PolygonMode polyMode;
   Extent extent;
   std::vector<std::string> shaders;
};

// ================================================================================================
// Conversion functions
VkFormat cydFormatToVkFormat( PixelFormat format );
VkColorSpaceKHR cydSpaceToVkSpace( ColorSpace space );
VkAttachmentLoadOp cydOpToVkOp( LoadOp op );
VkAttachmentStoreOp cydOpToVkOp( StoreOp op );
VkPrimitiveTopology cydDrawPrimToVkDrawPrim( DrawPrimitive prim );
VkPolygonMode cydPolyModeToVkPolyMode( PolygonMode polyMode );
}

template <>
struct std::hash<cyd::Extent>
{
   size_t operator()( const cyd::Extent& extent ) const
   {
      size_t seed = 0;
      hash_combine( seed, extent.width );
      hash_combine( seed, extent.height );
      return seed;
   }
};

template <>
struct std::hash<cyd::Attachment>
{
   size_t operator()( const cyd::Attachment& attachment ) const
   {
      size_t seed = 0;
      hash_combine( seed, attachment.format );
      hash_combine( seed, attachment.loadOp );
      hash_combine( seed, attachment.storeOp );
      hash_combine( seed, attachment.type );
      hash_combine( seed, attachment.usage );

      return seed;
   }
};

template <>
struct std::hash<cyd::RenderPassInfo>
{
   size_t operator()( const cyd::RenderPassInfo& renderPass ) const
   {
      const std::vector<cyd::Attachment>& attachments = renderPass.attachments;

      size_t seed = 0;
      for( const auto& attachment : attachments )
      {
         hash_combine( seed, attachment );
      }
      return seed;
   }
};

template <>
struct std::hash<cyd::PipelineLayoutInfo>
{
   size_t operator()( const cyd::PipelineLayoutInfo& pipLayoutInfo ) const
   {
      size_t seed = 0;
      hash_combine( seed, pipLayoutInfo.dummy );
      return seed;
   }
};

template <>
struct std::hash<cyd::PipelineInfo>
{
   size_t operator()( const cyd::PipelineInfo& pipInfo ) const
   {
      const std::vector<std::string>& shaders = pipInfo.shaders;

      size_t seed = 0;
      hash_combine( seed, pipInfo.pipLayout );
      hash_combine( seed, pipInfo.drawPrim );
      hash_combine( seed, pipInfo.polyMode );
      hash_combine( seed, pipInfo.extent );
      for( const auto& shader : shaders )
      {
         hash_combine( seed, shader );
      }

      return seed;
   }
};
