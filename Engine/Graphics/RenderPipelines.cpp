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
   if( typeString == "IMAGE" )
   {
      return ShaderResourceType::STORAGE_IMAGE;
   }
   if( typeString == "BUFFER" )
   {
      return ShaderResourceType::STORAGE;
   }

   CYDASSERT( !"Pipelines: Could not recognize string as a shader resource type" );
   return ShaderResourceType::UNIFORM;
}

static ShaderStageFlag StringToShaderStageFlags( const std::string& stagesString )
{
   ShaderStageFlag shaderStages = 0;

   if( stagesString.find( "VERTEX" ) != std::string::npos )
   {
      shaderStages |= ShaderStage::VERTEX_STAGE;
   }
   if( stagesString.find( "FRAGMENT" ) != std::string::npos )
   {
      shaderStages |= ShaderStage::FRAGMENT_STAGE;
   }
   if( stagesString.find( "COMPUTE" ) != std::string::npos )
   {
      shaderStages |= ShaderStage::COMPUTE_STAGE;
   }

   return shaderStages;
}

static DrawPrimitive StringToPrimitiveType( const std::string& primString )
{
   if( primString == "TRIANGLES" )
   {
      return DrawPrimitive::TRIANGLES;
   }
   if( primString == "TRIANGLE_STRIPS" )
   {
      return DrawPrimitive::TRIANGLE_STRIPS;
   }

   CYDASSERT( !"Pipelines: Could not recognize string as a primitive draw type" );
   return DrawPrimitive::TRIANGLES;
}

static PolygonMode StringToPolygonMode( const std::string& polyModeString )
{
   if( polyModeString == "FILL" )
   {
      return PolygonMode::FILL;
   }
   if( polyModeString == "LINE" )
   {
      return PolygonMode::LINE;
   }
   if( polyModeString == "POINT" )
   {
      return PolygonMode::POINT;
   }

   CYDASSERT( !"Pipelines: Could not recognize string as a polygon mode" );
   return PolygonMode::FILL;
}

static PixelFormat StringToPixelFormat( const std::string& formatString )
{
   if( formatString == "RGBA32F" )
   {
      return PixelFormat::RGBA32F;
   }
   if( formatString == "RGB32F" )
   {
      return PixelFormat::RGB32F;
   }

   CYDASSERT( !"Pipelines: Could not recognize string as a pixel format" );
   return PixelFormat::RGBA32F;
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
         if( primIt != pipeline.end() )
         {
            pipInfo.drawPrim = StringToPrimitiveType( primIt->front() );
         }
         else
         {
            pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
         }

         const auto& polyModeIt = pipeline.find( "POLYGON_MODE" );
         if( polyModeIt == pipeline.end() )
         {
            pipInfo.polyMode = PolygonMode::FILL;
         }
         else
         {
            pipInfo.polyMode = StringToPolygonMode( polyModeIt->front() );
         }

         // Parsing vertex layout
         const auto& vertLayoutIt = pipeline.find( "VERTEX_LAYOUT" );
         if( vertLayoutIt != pipeline.end() )
         {
            uint32_t curOffset = 0;
            for( const auto& attribute : *vertLayoutIt )
            {
               const uint32_t location     = attribute["LOCATION"];
               const PixelFormat vecFormat = StringToPixelFormat( attribute["FORMAT"] );
               uint32_t offset             = 0;
               uint32_t binding            = 0;

               // Optionals
               const auto& offsetIt = attribute.find( "OFFSET" );
               if( offsetIt == attribute.end() )
               {
                  offset = curOffset;
                  curOffset += GetPixelSizeInBytes( vecFormat );
               }
               else
               {
                  offset = offsetIt->front();
                  curOffset += offset;
               }

               const auto& bindingIt = attribute.find( "BINDING" );
               if( bindingIt != attribute.end() )
               {
                  binding = bindingIt->front();
               }

               pipInfo.vertLayout.addAttribute( vecFormat, location, offset, binding );
            }
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

      // Parsing shader resource sets and bindings
      const auto& resourcesIt = pipeline.find( "SHADER_RESOURCES" );
      if( resourcesIt != pipeline.end() )
      {
         for( const auto& resource : *resourcesIt )
         {
            const std::string resourceType = resource["TYPE"];
            if( resourceType == "CONSTANT_BUFFER" )
            {
               const PushConstantRange constantBuffer = {
                   StringToShaderStageFlags( resource["STAGE"] ), 0, resource["SIZE"] };

               pipIt->pipLayout.ranges.push_back( constantBuffer );
            }
            else
            {
               const uint32_t set = resource["SET"];

               ShaderBindingInfo bindingInfo = {};
               bindingInfo.name              = resource["NAME"];
               bindingInfo.stages            = StringToShaderStageFlags( resource["STAGE"] );
               bindingInfo.type              = StringToResourceType( resource["TYPE"] );
               bindingInfo.binding           = resource["BINDING"];

               pipIt->pipLayout.shaderSets[set].shaderBindings.push_back( bindingInfo );
            }
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
