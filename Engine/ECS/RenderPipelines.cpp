#include <ECS/RenderPipelines.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/Pipelines.h>

#include <ECS/Components/Rendering/CustomRenderableComponent.h>
#include <ECS/Components/Rendering/PhongRenderableComponent.h>
#include <ECS/Components/Rendering/PBRRenderableComponent.h>

namespace CYD
{
static DescriptorSetLayoutInfo environmentSet;

namespace CustomPipeline
{
static GraphicsPipelineInfo pipInfo;
static constexpr char VERTEX_SHADER[]   = "PASSTHROUGH_VERT";
static constexpr char FRAGMENT_SHADER[] = "PASSTHROUGH_FRAG";

void Initialize()
{
   const PushConstantRange timeConstant = { ShaderStage::FRAGMENT_STAGE, 0, sizeof( float ) };

   pipInfo.pipLayout.ranges.push_back( timeConstant );

   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = { 1920, 1080 };
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = { VERTEX_SHADER, FRAGMENT_SHADER };
}

void Render( CmdListHandle cmdList, const double deltaS, const RenderableComponent* renderable )
{
   static double timeElapsed = 0;
   timeElapsed += deltaS;

   const CustomRenderableComponent& customRenderable =
       *static_cast<const CustomRenderableComponent*>( renderable );

   pipInfo.shaders[1] = customRenderable.fragShader;

   GRIS::BindPipeline( cmdList, pipInfo );
   GRIS::UpdateConstantBuffer(
       cmdList, ShaderStage::FRAGMENT_STAGE, 0, sizeof( float ), &timeElapsed );
}
}

namespace DefaultPipeline
{
static GraphicsPipelineInfo pipInfo;
static constexpr char VERTEX_SHADER[]   = "DEFAULT_VERT";
static constexpr char FRAGMENT_SHADER[] = "DEFAULT_FRAG";

void Initialize()
{
   pipInfo.pipLayout.descSets.push_back( environmentSet );  // Set 0

   const PushConstantRange modelConstant = {
       ShaderStage::VERTEX_STAGE, 0, 3 * sizeof( glm::mat4 ) };

   pipInfo.pipLayout.ranges.push_back( modelConstant );

   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = { 1920, 1080 };  // TODO yikes
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = { VERTEX_SHADER, FRAGMENT_SHADER };
}

void Render(
    CmdListHandle cmdList,
    const glm::mat4& modelMatrix,
    const RenderableComponent* /*renderable*/ )
{
   GRIS::BindPipeline( cmdList, pipInfo );
   GRIS::UpdateConstantBuffer(
       cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( glm::mat4 ), &modelMatrix );
}
}

namespace PhongPipeline
{
static GraphicsPipelineInfo pipInfo;
static constexpr char VERTEX_SHADER[]   = "PHONG_TEX_VERT";
static constexpr char FRAGMENT_SHADER[] = "PHONG_TEX_FRAG";

void Initialize()
{
   DescriptorSetLayoutInfo materialSet;

   const ShaderResourceInfo texInfo = {
       ShaderResourceType::COMBINED_IMAGE_SAMPLER, ShaderStage::FRAGMENT_STAGE, 0 };

   materialSet.shaderResources.push_back( texInfo );

   const PushConstantRange modelConstant = { ShaderStage::VERTEX_STAGE, 0, sizeof( glm::mat4 ) };

   pipInfo.pipLayout.descSets.push_back( environmentSet );  // Set 0
   pipInfo.pipLayout.descSets.push_back( materialSet );     // Set 1

   pipInfo.pipLayout.ranges.push_back( modelConstant );

   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = { 1920, 1080 };  // TODO yikes
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = { VERTEX_SHADER, FRAGMENT_SHADER };
}

void Render(
    CmdListHandle cmdList,
    const glm::mat4& modelMatrix,
    const RenderableComponent* renderable )
{
   const PhongRenderableComponent& phongRenderable =
       *static_cast<const PhongRenderableComponent*>( renderable );

   GRIS::BindPipeline( cmdList, pipInfo );
   GRIS::BindTexture( cmdList, phongRenderable.texture, MATERIAL, 0 );
   GRIS::UpdateConstantBuffer(
       cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( glm::mat4 ), &modelMatrix );
}
}

namespace PBRPipeline
{
static GraphicsPipelineInfo pipInfo;
static constexpr char VERTEX_SHADER[]   = "PBR_TEX_VERT";
static constexpr char FRAGMENT_SHADER[] = "PBR_TEX_FRAG";

void Initialize()
{
   DescriptorSetLayoutInfo materialSet;

   ShaderResourceInfo texInfo = {
       ShaderResourceType::COMBINED_IMAGE_SAMPLER, ShaderStage::FRAGMENT_STAGE, MATERIAL };

   // PBR Maps
   // Albedo, normal, metallic, roughness, ambient occlusion
   for( uint32_t i = 0; i < 5; ++i )
   {
      texInfo.binding = i;
      materialSet.shaderResources.push_back( texInfo );
   }

   texInfo.binding = 5;
   texInfo.stages  = ShaderStage::VERTEX_STAGE;
   materialSet.shaderResources.push_back( texInfo );  // Heightmap

   const PushConstantRange modelRange = { ShaderStage::VERTEX_STAGE, 0, sizeof( glm::mat4 ) };

   pipInfo.pipLayout.descSets.push_back( environmentSet );  // Set 0
   pipInfo.pipLayout.descSets.push_back( materialSet );     // Set 1

   pipInfo.pipLayout.ranges.push_back( modelRange );

   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = { 1920, 1080 };  // TODO yikes
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = { VERTEX_SHADER, FRAGMENT_SHADER };
}

void Render(
    CmdListHandle cmdList,
    const glm::mat4& modelMatrix,
    const RenderableComponent* renderable )
{
   const PBRRenderableComponent& pbrRenderable =
       *static_cast<const PBRRenderableComponent*>( renderable );

   const bool hasAo     = pbrRenderable.ambientOcclusionMap != Handle::INVALID_HANDLE;
   const bool hasHeight = pbrRenderable.heightMap != Handle::INVALID_HANDLE;

   GRIS::BindPipeline( cmdList, pipInfo );

   GRIS::BindTexture( cmdList, pbrRenderable.albedo, MATERIAL, 0 );
   GRIS::BindTexture( cmdList, pbrRenderable.normalMap, MATERIAL, 1 );
   GRIS::BindTexture( cmdList, pbrRenderable.metalnessMap, MATERIAL, 2 );
   GRIS::BindTexture( cmdList, pbrRenderable.roughnessMap, MATERIAL, 3 );

   // Optionals
   if( hasAo )
   {
      GRIS::BindTexture( cmdList, pbrRenderable.ambientOcclusionMap, MATERIAL, 4 );
   }

   if( hasHeight )
   {
      GRIS::BindTexture( cmdList, pbrRenderable.heightMap, MATERIAL, 5 );
   }

   GRIS::UpdateConstantBuffer(
       cmdList, ShaderStage::VERTEX_STAGE, 0, sizeof( glm::mat4 ), &modelMatrix );
}
}

namespace RenderPipelines
{
bool Initialize()
{
   const ShaderResourceInfo viewProjectionInfo = {
       ShaderResourceType::UNIFORM, ShaderStage::VERTEX_STAGE, 0 };
   const ShaderResourceInfo positionInfo = {
       ShaderResourceType::UNIFORM, ShaderStage::FRAGMENT_STAGE, 1 };
   const ShaderResourceInfo lightInfo = {
       ShaderResourceType::UNIFORM, ShaderStage::FRAGMENT_STAGE, 2 };

   environmentSet.shaderResources.push_back( viewProjectionInfo );
   environmentSet.shaderResources.push_back( positionInfo );
   environmentSet.shaderResources.push_back( lightInfo );

   CustomPipeline::Initialize();
   DefaultPipeline::Initialize();
   PhongPipeline::Initialize();
   PBRPipeline::Initialize();

   return true;
}
}
}
