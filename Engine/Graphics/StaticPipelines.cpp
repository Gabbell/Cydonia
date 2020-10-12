#include <Graphics/StaticPipelines.h>

#include <Common/Assert.h>

#include <Graphics/PipelineInfos.h>

#include <json/json.hpp>

#include <array>
#include <fstream>
#include <memory>

namespace CYD::StaticPipelines
{
static constexpr char PIPELINES_PATH[] = "Data/Pipelines/Pipelines.json";

static std::array<PipelineInfo*, static_cast<size_t>( StaticPipelines::Type::COUNT )> s_pipelines;

static ShaderResourceType StringToResourceType( const std::string& typeString )
{
   if( typeString == "UBO" )
   {
      return ShaderResourceType::UNIFORM;
   }
   if( typeString == "SAMPLER" )
   {
      return ShaderResourceType::COMBINED_IMAGE_SAMPLER;
   }

   CYDASSERT( !"Pipelines: Could not recognize string as a shader resource type" );
   return ShaderResourceType::UNIFORM;
}

static ShaderStage::ShaderStage StringToShaderStage( const std::string& stageString )
{
   if( stageString == "VERTEX" )
   {
      return ShaderStage::VERTEX_STAGE;
   }
   if( stageString == "FRAGMENT" )
   {
      return ShaderStage::FRAGMENT_STAGE;
   }
   if( stageString == "COMPUTE" )
   {
      return ShaderStage::COMPUTE_STAGE;
   }

   CYDASSERT( !"Pipelines: Could not recognize string as a shader stage" );
   return ShaderStage::VERTEX_STAGE;
}

bool Initialize()
{
   // Parse pipeline infos from JSON description
   nlohmann::json pipelineDescriptions;
   std::ifstream( PIPELINES_PATH ) >> pipelineDescriptions;

   const auto& pipelines = pipelineDescriptions.front();

   // A pipeline is what is used during a render stage
   for( const auto& pipeline : pipelines )
   {
      const uint32_t pipIndex = pipeline["INDEX"];

      const PipelineInfo* attempt = s_pipelines[pipIndex];
      if( attempt != nullptr )
      {
         CYDASSERT( !"Pipelines: Index already taken, ignoring" );
         continue;
      }

      auto& pipIt = s_pipelines[pipIndex];

      const std::string pipelineType = pipeline["TYPE"];
      if( pipelineType == "GRAPHICS" )  // GRAPHICS PIPELINE
      {
         GraphicsPipelineInfo pipInfo;
         pipInfo.shaders.push_back( pipeline["VERTEX_SHADER"] );
         pipInfo.shaders.push_back( pipeline["FRAGMENT_SHADER"] );

         // Optional parameters
         const auto& primIt = pipeline.find( "PRIMITIVE" );
         if( primIt == pipeline.end() )
         {
            pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
         }
         else
         {
            // TODO
         }

         const auto& polyModeIt = pipeline.find( "POLYGON_MODE" );
         if( polyModeIt == pipeline.end() )
         {
            pipInfo.polyMode = PolygonMode::FILL;
         }
         else
         {
            // TODO
         }

         pipIt = new GraphicsPipelineInfo( pipInfo );
      }
      else if( pipelineType == "COMPUTE" )  // COMPUTE PIPELINE
      {
         ComputePipelineInfo pipInfo;
         pipInfo.shader = pipeline["COMPUTE_SHADER"];

         pipIt = new ComputePipelineInfo( pipInfo );
      }
      else
      {
         CYDASSERT( !"Pipelines: Pipeline type string not recognized" );
         continue;
      }

      std::map<uint32_t, DescriptorSetLayoutInfo> sets;
      for( const auto& input : pipeline["INPUTS"] )
      {
         const std::string resourceType = input["TYPE"];
         if( resourceType == "CONSTANT_BUFFER" )
         {
            const PushConstantRange constantBuffer = {
                StringToShaderStage( input["STAGE"] ), 0, input["SIZE"]};

            pipIt->pipLayout.ranges.push_back( constantBuffer );
         }
         else
         {
            ShaderResourceInfo resourceInfo = {};
            resourceInfo.stages             = StringToShaderStage( input["STAGE"] );
            resourceInfo.type               = StringToResourceType( input["TYPE"] );
            resourceInfo.set                = input["SET"];
            resourceInfo.binding            = input["BINDING"];

            sets[resourceInfo.set].shaderResources.push_back( resourceInfo );
         }
      }
      for( const auto& setPair : sets )
      {
         pipIt->pipLayout.descSets.push_back( setPair.second );
      }
   }

   return true;
}

void Uninitialize()
{
   for( auto& pipeline : s_pipelines )
   {
      delete pipeline;
   }
}

const PipelineInfo* Get( StaticPipelines::Type type )
{
   return s_pipelines[static_cast<uint32_t>( type )];
}
}
