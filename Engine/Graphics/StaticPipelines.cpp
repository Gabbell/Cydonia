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
   if( typeString == "PUSH_CONSTANT" )
   {
      return ShaderResourceType::PUSH_CONSTANT;
   }
   if( typeString == "UNIFORM" )
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
   if( stagesString.find( "TESS_CONTROL" ) != std::string::npos )
   {
      shaderStages |= PipelineStage::TESS_CONTROL_STAGE;
   }
   if( stagesString.find( "TESS_EVAL" ) != std::string::npos )
   {
      shaderStages |= PipelineStage::TESS_EVAL_STAGE;
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
   if( primString == "PATCHES" )
   {
      return DrawPrimitive::PATCHES;
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

static VertexLayout::Attribute StringToVertexAttribute( const std::string& attributeString )
{
   if( attributeString == "POSITION" )
   {
      return VertexLayout::Attribute::POSITION;
   }

   if( attributeString == "NORMAL" )
   {
      return VertexLayout::Attribute::NORMAL;
   }

   if( attributeString == "TEXCOORD" )
   {
      return VertexLayout::Attribute::TEXCOORD;
   }

   if( attributeString == "TANGENT" )
   {
      return VertexLayout::Attribute::TANGENT;
   }

   if( attributeString == "BITANGENT" )
   {
      return VertexLayout::Attribute::BITANGENT;
   }

   if( attributeString == "COLOR" )
   {
      return VertexLayout::Attribute::COLOR;
   }

   CYD_ASSERT( !"Pipelines: Could not recognize string as a vertex attribute" );
   return VertexLayout::Attribute::POSITION;
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

static CullMode StringToCullMode( const std::string& cullModeString )
{
   if( cullModeString == "NONE" )
   {
      return CullMode::NONE;
   }

   if( cullModeString == "BACK" )
   {
      return CullMode::BACK;
   }

   if( cullModeString == "FRONT" )
   {
      return CullMode::FRONT;
   }

   if( cullModeString == "FRONT_AND_BACK" )
   {
      return CullMode::FRONT_AND_BACK;
   }

   CYD_ASSERT( !"Pipelines: Could not recognize string as a cull mode" );
   return CullMode::NONE;
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

         if( pipeline.contains( "VERTEX_SHADER" ) )
         {
            pipInfo.shaders.push_back( pipeline["VERTEX_SHADER"] );
         }

         if( pipeline.contains( "TESS_CONTROL_SHADER" ) )
         {
            pipInfo.shaders.push_back( pipeline["TESS_CONTROL_SHADER"] );
         }

         if( pipeline.contains( "TESS_EVAL_SHADER" ) )
         {
            pipInfo.shaders.push_back( pipeline["TESS_EVAL_SHADER"] );
         }

         if( pipeline.contains( "FRAGMENT_SHADER" ) )
         {
            pipInfo.shaders.push_back( pipeline["FRAGMENT_SHADER"] );
         }

         CYD_ASSERT( !pipInfo.shaders.empty() && "Pipeline: No shaders were added" );

         // Optional parameters
         if( pipeline.contains( "PRIMITIVE" ) )
         {
            pipInfo.rasterizer.drawPrim = StringToPrimitiveType( pipeline["PRIMITIVE"] );
         }

         // Parsing vertex layout
         if( pipeline.contains( "VERTEX_LAYOUT" ) )
         {
            for( const auto& attribute : pipeline["VERTEX_LAYOUT"] )
            {
               const PixelFormat vecFormat = StringToPixelFormat( attribute["FORMAT"] );

               const VertexLayout::Attribute attributeType =
                   StringToVertexAttribute( attribute["ATTRIBUTE"] );

               pipInfo.vertLayout.addAttribute( attributeType, vecFormat );
            }
         }

         // Parsing depth stencil state
         if( pipeline.contains( "DEPTH_STENCIL" ) )
         {
            const auto& depthStencil       = pipeline["DEPTH_STENCIL"];
            pipInfo.dsState.useDepthTest   = depthStencil["DEPTH_TEST_ENABLE"];
            pipInfo.dsState.useStencilTest = depthStencil["STENCIL_TEST_ENABLE"];
            pipInfo.dsState.depthWrite     = depthStencil["DEPTH_WRITE_ENABLE"];
            pipInfo.dsState.depthCompareOp = StringToCompareOp( depthStencil["DEPTH_COMPARE_OP"] );
         }

         // Parsing tessellation state
         if( pipeline.contains( "TESSELLATION" ) )
         {
            const auto& tessellation             = pipeline["TESSELLATION"];
            pipInfo.tessState.enabled            = tessellation["ENABLE"];
            pipInfo.tessState.patchControlPoints = tessellation["PATCH_VERTICES"];
         }

         // Parsing rasterizer state
         if( pipeline.contains( "RASTERIZER" ) )
         {
            const auto& rasterizer = pipeline["RASTERIZER"];

            if( rasterizer.contains( "PRIMITIVE" ) )
            {
               pipInfo.rasterizer.drawPrim = StringToPrimitiveType( rasterizer["PRIMITIVE"] );
            }

            if( rasterizer.contains( "CULL_MODE" ) )
            {
               pipInfo.rasterizer.cullMode = StringToCullMode( rasterizer["CULL_MODE"] );
            }

            if( rasterizer.contains( "DEPTH_CLAMP" ) )
            {
               pipInfo.rasterizer.useDepthClamp = rasterizer["DEPTH_CLAMP"];
            }

            if( rasterizer.contains( "POLYGON_MODE" ) )
            {
               pipInfo.rasterizer.polyMode = StringToPolygonMode( rasterizer["POLYGON_MODE"] );
            }

            if( rasterizer.contains( "PRIMITIVE_RESTART" ) )
            {
               pipInfo.rasterizer.usePrimitiveRestart = rasterizer["PRIMITIVE_RESTART"];
            }

            if( rasterizer.contains( "DEPTH_BIAS_ENABLE" ) )
            {
               pipInfo.rasterizer.useDepthBias        = rasterizer["DEPTH_BIAS_ENABLE"];
               pipInfo.rasterizer.depthBiasConstant   = rasterizer["DEPTH_CONSTANT"];
               pipInfo.rasterizer.depthBiasSlopeScale = rasterizer["DEPTH_SLOPE_SCALE"];
            }
         }

         if( pipeline.contains( "BLEND" ) )
         {
            const auto& blend = pipeline["BLEND"];

            pipInfo.blendState.useBlend = blend["BLEND_ENABLE"];
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
      if( pipeline.contains( "SHADER_RESOURCES" ) )
      {
         for( const auto& resource : pipeline["SHADER_RESOURCES"] )
         {
            CYD::PipelineStageFlag stages = 0;
            for( const auto& shaderStage : resource["STAGES"] )
            {
               stages |= StringToShaderStageFlags( shaderStage );
            }

            const ShaderResourceType resourceType = StringToResourceType( resource["TYPE"] );
            if( resourceType == ShaderResourceType::PUSH_CONSTANT )
            {
               uint32_t offset = 0;
               if( resource.find( "OFFSET" ) != resource.end() )
               {
                  offset = resource["OFFSET"];
               }

               const PushConstantRange constantBuffer = {
                   resource["NAME"], stages, offset, resource["SIZE"] };

               newPipeline->pipLayout.ranges.push_back( constantBuffer );
            }
            else
            {
               const uint32_t set     = resource["SET"];
               const uint32_t binding = resource["BINDING"];

               newPipeline->pipLayout.addBinding(
                   resourceType, stages, binding, set, resource["NAME"] );
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
