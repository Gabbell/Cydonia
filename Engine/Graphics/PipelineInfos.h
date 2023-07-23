#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/VertexLayout.h>
#include <Graphics/ShaderConstants.h>

#include <string>
#include <string_view>
#include <vector>

namespace CYD
{
struct PushConstantRange
{
   bool operator==( const PushConstantRange& other ) const;

   std::string name;
   PipelineStageFlag stages;
   size_t offset;
   size_t size;
};

struct ShaderBindingInfo
{
   bool operator==( const ShaderBindingInfo& other ) const;

   std::string name;
   ShaderResourceType type;
   PipelineStageFlag stages;
   uint32_t offset;
   uint32_t range;
   uint8_t binding;
};

struct ShaderSetInfo
{
   bool operator==( const ShaderSetInfo& other ) const;

   uint8_t set;
   std::vector<ShaderBindingInfo> shaderBindings;
};

struct RasterizerState
{
   bool useDepthBias         = false;
   float depthBiasConstant   = 0.0f;
   float depthBiasSlopeScale = 0.0f;
};

struct DepthStencilState
{
   bool useDepthTest              = false;
   bool useStencilTest            = false;
   bool depthWrite                = false;
   CompareOperator depthCompareOp = CompareOperator::ALWAYS;
};

struct BlendState
{
   bool useBlend = false;
};

struct TessellationState
{
   bool enabled                = false;
   uint32_t patchControlPoints = 0;
};

struct PipelineLayoutInfo
{
   bool operator==( const PipelineLayoutInfo& other ) const;

   void addBinding(
       ShaderResourceType type,
       PipelineStageFlag stages,
       uint8_t binding,
       uint8_t set                 = 0,
       const std::string_view name = "" );

   std::vector<PushConstantRange> ranges;
   std::vector<ShaderSetInfo> shaderSets;
};

// Flat shader resource binding
template <class T>
struct FlatShaderBinding
{
   FlatShaderBinding() = default;
   FlatShaderBinding(
       T* resource,
       CYD::ShaderResourceType type,
       uint8_t binding,
       uint8_t set,
       size_t offset = 0,
       size_t range  = 0 )
       : resource( resource ),
         type( type ),
         binding( binding ),
         set( set ),
         offset( offset ),
         range( range )
   {
   }

   T* resource;
   CYD::ShaderResourceType type;
   uint8_t binding;
   uint8_t set;
   size_t offset;
   size_t range;
   bool valid;
};

// Should not create unspecialized version of this struct
struct PipelineInfo
{
   virtual ~PipelineInfo();

   std::string name;
   PipelineType type;
   PipelineLayoutInfo pipLayout;
   ShaderConstants constants;

   const PushConstantRange* findPushConstant( const std::string_view name ) const
   {
      for( const PushConstantRange& range : pipLayout.ranges )
      {
         if( range.name == name )
         {
            return &range;
         }
      }

      return nullptr;
   }

   template <class T>
   FlatShaderBinding<T> findBinding( T resource, const std::string_view name ) const
   {
      FlatShaderBinding<T> binding = {};

      for( const ShaderSetInfo& shaderSetInfo : pipLayout.shaderSets )
      {
         for( const ShaderBindingInfo& shaderBindingInfo : shaderSetInfo.shaderBindings )
         {
            if( shaderBindingInfo.name == name )
            {
               // We found the binding
               binding.type    = shaderBindingInfo.type;
               binding.binding = shaderBindingInfo.binding;
               binding.set     = shaderSetInfo.set;

               if( resource )
               {
                  binding.valid = true;
               }
            }
         }
      }

      return binding;
   }

  protected:
   PipelineInfo();
   PipelineInfo( PipelineType type ) : type( type ) {}
};

// Specialized pipeline infos
struct GraphicsPipelineInfo final : public PipelineInfo
{
   GraphicsPipelineInfo();
   COPIABLE( GraphicsPipelineInfo );
   virtual ~GraphicsPipelineInfo();

   bool operator==( const GraphicsPipelineInfo& other ) const;

   std::vector<std::string> shaders;
   VertexLayout vertLayout;
   DepthStencilState dsState;
   BlendState blendState;
   TessellationState tessState;
   RasterizerState rasterizer;
   DrawPrimitive drawPrim;
   PolygonMode polyMode;
   Extent2D extent;
};

struct ComputePipelineInfo final : public PipelineInfo
{
   ComputePipelineInfo();
   COPIABLE( ComputePipelineInfo );
   virtual ~ComputePipelineInfo();

   bool operator==( const ComputePipelineInfo& other ) const;

   std::string shader;
};
}

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
struct std::hash<CYD::GraphicsPipelineInfo>
{
   size_t operator()( const CYD::GraphicsPipelineInfo& pipInfo ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, pipInfo.type );
      hashCombine( seed, pipInfo.pipLayout );
      hashCombine( seed, pipInfo.drawPrim );
      hashCombine( seed, pipInfo.polyMode );
      hashCombine( seed, pipInfo.extent );
      for( const auto& shader : pipInfo.shaders )
      {
         hashCombine( seed, shader );
      }

      return seed;
   }
};

template <>
struct std::hash<CYD::ComputePipelineInfo>
{
   size_t operator()( const CYD::ComputePipelineInfo& pipInfo ) const noexcept
   {
      size_t seed = 0;
      hashCombine( seed, pipInfo.type );
      hashCombine( seed, pipInfo.pipLayout );
      hashCombine( seed, pipInfo.shader );

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