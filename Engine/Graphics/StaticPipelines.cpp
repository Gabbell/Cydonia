#include <Graphics/StaticPipelines.h>

#include <Common/Assert.h>

#include <Graphics/PipelineInfos.h>

#include <json/json.hpp>

#include <fstream>
#include <memory>

namespace CYD::StaticPipelines
{
static constexpr char PIPELINES_PATH[] = "Pipelines.json";

static constexpr uint32_t MAX_STATIC_PIPELINES = 128;
static std::array<PipelineInfo*, MAX_STATIC_PIPELINES> s_pipelines;
static uint32_t s_pipelineCount = 0;

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

   CYD_ASSERT( !"Pipelines: Could not recognize string as a shader resource type" );
   return ShaderResourceType::UNIFORM;
}

static PipelineStageFlag StringToShaderStageFlags( const std::string& stagesString )
{
   PipelineStageFlag shaderStages = 0;

   if( stagesString.find( "VERTEX" ) != std::string::npos )
   {
      shaderStages |= PipelineStage::VERTEX_STAGE;
   }
   if( stagesString.find( "FRAGMENT" ) != std::string::npos )
   {
      shaderStages |= PipelineStage::FRAGMENT_STAGE;
   }
   if( stagesString.find( "COMPUTE" ) != std::string::npos )
   {
      shaderStages |= PipelineStage::COMPUTE_STAGE;
   }
   if( stagesString.find( "GRAPHICS" ) != std::string::npos )
   {
      shaderStages |= PipelineStage::ALL_GRAPHICS_STAGES;
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

   CYD_ASSERT( !"Pipelines: Could not recognize string as a primitive draw type" );
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

   CYD_ASSERT( !"Pipelines: Could not recognize string as a polygon mode" );
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

   CYD_ASSERT( !"Pipelines: Could not recognize string as a pixel format" );
   return PixelFormat::RGBA32F;
}

static CompareOperator StringToCompareOp( const std::string& operatorString )
{
   if( operatorString == "NEVER" )
   {
      return CompareOperator::NEVER;
   }
   if( operatorString == "LESS" )
   {
      return CompareOperator::LESS;
   }
   if( operatorString == "EQUAL" )
   {
      return CompareOperator::EQUAL;
   }
   if( operatorString == "LESS_EQUAL" )
   {
      return CompareOperator::LESS_EQUAL;
   }
   if( operatorString == "GREATER" )
   {
      return CompareOperator::GREATER;
   }
   if( operatorString == "GREATER_EQUAL" )
   {
      return CompareOperator::GREATER_EQUAL;
   }
   if( operatorString == "NOT_EQUAL" )
   {
      return CompareOperator::NOT_EQUAL;
   }
   if( operatorString == "ALWAYS" )
   {
      return CompareOperator::ALWAYS;
   }

   CYD_ASSERT( !"Pipelines: Could not recognize string as a compare operator" );
   return CompareOperator::ALWAYS;
}

bool Initialize()
{
   // Parse pipeline infos from JSON description
   nlohmann::json pipelineDescriptions;
   std::ifstream pipelinesFile( PIPELINES_PATH );

   if( !pipelinesFile.is_open() )
   {
      CYD_ASSERT( !"StaticPipelines: Could not find render pipelines file" );
      return false;
   }

   pipelinesFile >> pipelineDescriptions;

   const auto& pipelines = pipelineDescriptions.front();

   printf( "======== Initializing Static Pipelines ========\n" );

   // A pipeline is what is used during a render stage
   for( uint32_t pipIdx = 0; pipIdx < pipelines.size(); ++pipIdx )
   {
      PipelineInfo* newPipeline = nullptr;

      const auto& pipeline = pipelines[pipIdx];

      const std::string pipName = pipeline["NAME"];

      // Making sure there is no name duplication
#if CYD_ASSERTIONS_ENABLED
      bool shouldIgnore = false;
      for( uint32_t i = 0; i < s_pipelineCount; ++i )
      {
         const PipelineInfo* pipInfo = s_pipelines[i];

         CYD_ASSERT( pipInfo );
         if( pipInfo->name == pipName )
         {
            CYD_ASSERT( !"Pipelines: Name already taken, ignoring" );
            shouldIgnore = true;
         }
      }
      if( shouldIgnore )
      {
         continue;
      }
#endif

      const std::string pipelineType = pipeline["TYPE"];
      if( pipelineType == "GRAPHICS" )  // GRAPHICS PIPELINE
      {
         GraphicsPipelineInfo pipInfo;
         pipInfo.name = pipName;
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
         const auto& vertexLayoutIt = pipeline.find( "VERTEX_LAYOUT" );
         if( vertexLayoutIt != pipeline.end() )
         {
            uint32_t curOffset = 0;
            for( const auto& attribute : *vertexLayoutIt )
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

         // Parsing depth stencil state
         const auto& depthStencilIt = pipeline.find( "DEPTH_STENCIL" );
         if( depthStencilIt != pipeline.end() )
         {
            const auto& depthStencil       = *depthStencilIt;
            pipInfo.dsState.useDepthTest   = depthStencil["DEPTH_TEST_ENABLE"];
            pipInfo.dsState.useStencilTest = depthStencil["STENCIL_TEST_ENABLE"];
            pipInfo.dsState.depthWrite     = depthStencil["DEPTH_WRITE_ENABLE"];
            pipInfo.dsState.depthCompareOp = StringToCompareOp(depthStencil["DEPTH_COMPARE_OP"]);
         }

         // Parsing rasterizer state
         const auto& rasterizerIt = pipeline.find( "RASTERIZER" );
         if( rasterizerIt != pipeline.end() )
         {
            const auto& rasterizer                 = *rasterizerIt;
            pipInfo.rasterizer.useDepthBias        = rasterizer["DEPTH_BIAS_ENABLE"];
            pipInfo.rasterizer.depthBiasConstant   = rasterizer["DEPTH_CONSTANT"];
            pipInfo.rasterizer.depthBiasSlopeScale = rasterizer["DEPTH_SLOPE_SCALE"];
         }

         newPipeline = new GraphicsPipelineInfo( pipInfo );
      }
      else if( pipelineType == "COMPUTE" )  // COMPUTE PIPELINE
      {
         ComputePipelineInfo pipInfo;
         pipInfo.name   = pipName;
         pipInfo.shader = pipeline["COMPUTE_SHADER"];

         newPipeline = new ComputePipelineInfo( pipInfo );
      }
      else
      {
         CYD_ASSERT( !"Pipelines: Pipeline type string not recognized" );
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
               uint32_t offset = 0;
               if( resource.find( "OFFSET" ) != resource.end() )
               {
                  offset = resource["OFFSET"];
               }

               const PushConstantRange constantBuffer = {
                   StringToShaderStageFlags( resource["STAGE"] ), offset, resource["SIZE"] };

               newPipeline->pipLayout.ranges.push_back( constantBuffer );
            }
            else
            {
               const uint32_t set     = resource["SET"];
               const uint32_t binding = resource["BINDING"];

               newPipeline->pipLayout.addBinding(
                   StringToResourceType( resource["TYPE"] ),
                   StringToShaderStageFlags( resource["STAGE"] ),
                   binding,
                   set,
                   resource["NAME"] );
            }
         }
      }

      printf( "Added pipeline --> %s\n", pipName.c_str() );

      s_pipelines[pipIdx] = newPipeline;
      s_pipelineCount++;
   }

   printf( "======== End Static Pipelines ========\n" );

   return true;
}

void Uninitialize()
{
   for( PipelineInfo*& pip : s_pipelines )
   {
      delete pip;
   }
}

PipelineIndex FindByName( std::string_view pipName )
{
   for( uint32_t i = 0; i < s_pipelineCount; ++i )
   {
      const PipelineInfo* pipInfo = s_pipelines[i];
      if( pipInfo->name == pipName )
      {
         return i;
      }
   }

   return INVALID_PIPELINE_IDX;
}

const PipelineInfo* Get( PipelineIndex index )
{
   if( index < s_pipelineCount && index != INVALID_PIPELINE_IDX )
   {
      return s_pipelines[index];
   }

   return nullptr;
}
}
