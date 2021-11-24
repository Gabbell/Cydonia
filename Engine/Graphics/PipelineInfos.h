#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/VertexLayout.h>
#include <Graphics/ShaderConstants.h>

#include <string>
#include <vector>

namespace CYD
{
template <class T>
struct ResourceBinding
{
   ResourceBinding() = default;
   ResourceBinding( const T resource, CYD::ShaderResourceType type, uint32_t set, uint32_t binding )
       : resource( resource ), type( type ), set( set ), binding( binding )
   {
   }

   mutable T resource;
   CYD::ShaderResourceType type;
   uint32_t set;
   uint32_t binding;
   bool valid = false;
};

// Should not create unspecialized version of this struct
struct PipelineInfo
{
   virtual ~PipelineInfo();

   PipelineType type;
   PipelineLayoutInfo pipLayout;
   ShaderConstants constants;

   template <class T>
   ResourceBinding<T> findBinding( const T resource, const std::string_view name ) const
   {
      ResourceBinding<T> binding;

      for( const auto& shaderSetInfo : pipLayout.shaderSets )
      {
         for( const auto& shaderBindingInfo : shaderSetInfo.second.shaderBindings )
         {
            if( shaderBindingInfo.name == name )
            {
               // We found the binding
               binding.binding = shaderBindingInfo.binding;
               binding.type    = shaderBindingInfo.type;
               binding.set     = shaderSetInfo.first;

               // A resource binding is only valid if the resource is not null
               if( resource ) binding.valid = true;
            }
         }
      }

      binding.resource = resource;
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
   // PipelineState pipState; // TODO PSO hints
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
