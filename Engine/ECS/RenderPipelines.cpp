#include <ECS/RenderPipelines.h>

#include <Graphics/RenderInterface.h>

#include <ECS/Components/PhongRenderableComponent.h>
#include <ECS/Components/PBRRenderableComponent.h>

namespace cyd
{
namespace PhongPipeline
{
static constexpr char VERTEX_SHADER[]   = "phongTex_vert";
static constexpr char FRAGMENT_SHADER[] = "phongTex_frag";
static constexpr uint32_t MAX_LIGHTS    = 4;

static PipelineInfo pipInfo;

void init()
{
   DescriptorSetLayoutInfo environmentLayout;

   const ShaderResourceInfo viewInfo  = { ShaderResourceType::UNIFORM, VERTEX_STAGE, 0 };
   const ShaderResourceInfo lightInfo = { ShaderResourceType::UNIFORM, FRAGMENT_STAGE, 1 };

   environmentLayout.shaderResources.push_back( viewInfo );
   environmentLayout.shaderResources.push_back( lightInfo );

   DescriptorSetLayoutInfo materialLayout;

   const ShaderResourceInfo texInfo = {
       ShaderResourceType::COMBINED_IMAGE_SAMPLER, FRAGMENT_STAGE, 0 };

   materialLayout.shaderResources.push_back( texInfo );

   const PushConstantRange modelConstant = { VERTEX_STAGE, 0, sizeof( glm::mat4 ) };

   pipInfo.pipLayout.descSets.push_back( environmentLayout );  // Set 0
   pipInfo.pipLayout.descSets.push_back( materialLayout );     // Set 1

   pipInfo.pipLayout.ranges.push_back( modelConstant );

   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = { 1920, 1080 };  // TODO yikes
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = { VERTEX_SHADER, FRAGMENT_SHADER };

   pipInfo.constants.add( FRAGMENT_SHADER, 0, MAX_LIGHTS );
}

void render( CmdListHandle cmdList, const glm::mat4& model, const RenderableComponent* renderable )
{
   if( pipInfo.shaders.empty() )
   {
      init();
   }

   const PhongRenderableComponent& phongRenderable =
       *static_cast<const PhongRenderableComponent*>( renderable );

   GRIS::BindPipeline( cmdList, pipInfo );
   GRIS::BindTexture( cmdList, phongRenderable.texture, MATERIAL, 0 );
   GRIS::UpdateConstantBuffer( cmdList, VERTEX_STAGE, 0, sizeof( glm::mat4 ), &model );
}
}

namespace PBRPipeline
{
static constexpr char VERTEX_SHADER[]   = "pbrTex_vert";
static constexpr char FRAGMENT_SHADER[] = "pbrTex_frag";
static constexpr uint32_t MAX_LIGHTS    = 4;

static PipelineInfo pipInfo;

void init()
{
   DescriptorSetLayoutInfo environmentLayout;

   const ShaderResourceInfo viewInfo  = {ShaderResourceType::UNIFORM, VERTEX_STAGE, 0};
   const ShaderResourceInfo lightInfo = {ShaderResourceType::UNIFORM, FRAGMENT_STAGE, 1};

   environmentLayout.shaderResources.push_back( viewInfo );
   environmentLayout.shaderResources.push_back( lightInfo );

   DescriptorSetLayoutInfo materialLayout;

   ShaderResourceInfo texInfo = {
       ShaderResourceType::COMBINED_IMAGE_SAMPLER, FRAGMENT_STAGE, MATERIAL };

   // PBR Maps
   // Albedo, normal, metallic, roughness, ambient occlusion
   for( uint32_t i = 0; i < 5; ++i )
   {
      texInfo.binding = i;
      materialLayout.shaderResources.push_back( texInfo );
   }

   texInfo.binding = 5;
   texInfo.stages  = VERTEX_STAGE;
   materialLayout.shaderResources.push_back( texInfo );  // Heightmap

   const PushConstantRange modelRange = { VERTEX_STAGE, 0, sizeof( glm::mat4 ) };

   pipInfo.pipLayout.descSets.push_back( environmentLayout );  // Set 0
   pipInfo.pipLayout.descSets.push_back( materialLayout );     // Set 1

   pipInfo.pipLayout.ranges.push_back( modelRange );

   pipInfo.drawPrim = DrawPrimitive::TRIANGLES;
   pipInfo.extent   = { 1920, 1080 };  // TODO yikes
   pipInfo.polyMode = PolygonMode::FILL;
   pipInfo.shaders  = { VERTEX_SHADER, FRAGMENT_SHADER };

   pipInfo.constants.add( FRAGMENT_SHADER, 0, MAX_LIGHTS );
}

void render( CmdListHandle cmdList, const glm::mat4& model, const RenderableComponent* renderable )
{
   if( pipInfo.shaders.empty() )
   {
      init();
   }

   const PBRRenderableComponent& pbrRenderable =
       *static_cast<const PBRRenderableComponent*>( renderable );

   GRIS::BindPipeline( cmdList, pipInfo );
   GRIS::BindTexture( cmdList, pbrRenderable.albedo, MATERIAL, 0 );
   GRIS::BindTexture( cmdList, pbrRenderable.normalMap, MATERIAL, 1 );
   GRIS::BindTexture( cmdList, pbrRenderable.metalnessMap, MATERIAL, 2 );
   GRIS::BindTexture( cmdList, pbrRenderable.roughnessMap, MATERIAL, 3 );
   GRIS::BindTexture( cmdList, pbrRenderable.ambientOcclusionMap, MATERIAL, 4 );
   GRIS::BindTexture( cmdList, pbrRenderable.heightMap, MATERIAL, 5 );
   GRIS::UpdateConstantBuffer( cmdList, VERTEX_STAGE, 0, sizeof( glm::mat4 ), &model );
}
}
}
