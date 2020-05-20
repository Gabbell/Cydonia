#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/ShaderConstants.h>

#include <string>
#include <vector>

namespace CYD
{
// Should not create unspecialized version of this struct
struct PipelineInfo
{
   virtual ~PipelineInfo();

   PipelineType type;
   ShaderConstants constants;
   PipelineLayoutInfo pipLayout;

  protected:
   PipelineInfo();
   PipelineInfo( PipelineType type ) : type( type ) {}
};

// Specialized pipeline infos
struct GraphicsPipelineInfo final : public PipelineInfo
{
   GraphicsPipelineInfo();
   COPIABLE( GraphicsPipelineInfo )
   virtual ~GraphicsPipelineInfo();

   bool operator==( const GraphicsPipelineInfo& other ) const;

   std::vector<std::string> shaders;
   DrawPrimitive drawPrim;
   PolygonMode polyMode;
   Extent extent;
};

struct ComputePipelineInfo final : public PipelineInfo
{
   ComputePipelineInfo();
   COPIABLE( ComputePipelineInfo )
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
