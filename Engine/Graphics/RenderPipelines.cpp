#include <Graphics/RenderPipelines.h>

#include <Common/Assert.h>

#include <Graphics/PipelineInfos.h>

#include <json/json.hpp>

#include <fstream>
#include <memory>

namespace CYD::RenderPipelines
{
static constexpr char PIPELINES_PATH[] = "Data/Pipelines/RenderPipelines.json";

static std::unordered_map<std::string, PipelineInfo*> s_pipelines;

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
   std::ifstream pipelinesFile( PIPELINES_PATH );

   if( !pipelinesFile.is_open() )
   {
      CYDASSERT( !"RenderPipelines: Could not find render pipelines file" );
      return false;
   }

   pipelinesFile >> pipelineDescriptions;

   const auto& pipelines = pipelineDescriptions.front();

   // A pipeline is what is used during a render stage
   for( const auto& pipeline : pipelines )
   {
      const std::string pipName = pipeline["NAME"];

      const auto& attemptIt = s_pipelines.find( pipName );
      if( attemptIt != s_pipelines.end() )
      {
         CYDASSERT( !"Pipelines: Name already taken, ignoring" );
         continue;
      }

      auto& pipIt = s_pipelines[pipName];

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

      for( const auto& resource : pipeline["SHADER_RESOURCES"] )
      {
         const std::string resourceType = resource["TYPE"];
         if( resourceType == "CONSTANT_BUFFER" )
         {
            const PushConstantRange constantBuffer = {
                StringToShaderStage( resource["STAGE"] ), 0, resource["SIZE"]};

            pipIt->pipLayout.ranges.push_back( constantBuffer );
         }
         else
         {
            const uint32_t set = resource["SET"];

            ShaderBindingInfo bindingInfo = {};
            bindingInfo.name              = resource["NAME"];
            bindingInfo.stages            = StringToShaderStage( resource["STAGE"] );
            bindingInfo.type              = StringToResourceType( resource["TYPE"] );
            bindingInfo.binding           = resource["BINDING"];

            pipIt->pipLayout.shaderSets[set].shaderBindings.push_back( bindingInfo );
         }
      }
   }

   return true;
}

void Uninitialize()
{
   for( auto& pipIt : s_pipelines )
   {
      delete pipIt.second;
   }
}

const PipelineInfo* Get( std::string_view pipName )
{
   const auto& it = s_pipelines.find( std::string( pipName ) );
   if( it == s_pipelines.end() )
   {
      CYDASSERT( !"RenderPipelines: Could not find pipeline" );
      return nullptr;
   }

   return it->second;
}
}
