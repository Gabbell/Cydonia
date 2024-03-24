#include <ECS/Systems/Rendering/RenderSystem.h>

#include <Graphics/StaticPipelines.h>

#include <Graphics/GRIS/RenderHelpers.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/SharedComponents/SceneComponent.h>

#include <ECS/EntityManager.h>

namespace CYD
{
void RenderSystem::bindView(
    CmdListHandle cmdList,
    const SceneComponent& scene,
    const PipelineInfo* pipInfo,
    uint32_t viewIndex ) const
{
   CYD_ASSERT( pipInfo );

   const uint32_t viewOffset = viewIndex * sizeof( ViewShaderParams );

   GRIS::NamedBufferBinding(
       cmdList, scene.viewsBuffer, "Views", *pipInfo, viewOffset, sizeof( ViewShaderParams ) );
}

void RenderSystem::bindInverseView(
    CmdListHandle cmdList,
    const SceneComponent& scene,
    const PipelineInfo* pipInfo,
    uint32_t viewIndex ) const
{
   CYD_ASSERT( pipInfo );

   const uint32_t viewOffset = viewIndex * sizeof( InverseViewShaderParams );

   GRIS::NamedBufferBinding(
       cmdList,
       scene.inverseViewsBuffer,
       "InverseViews",
       *pipInfo,
       viewOffset,
       sizeof( InverseViewShaderParams ) );
}
}