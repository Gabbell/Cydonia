#include <ECS/Systems/Rendering/RenderSystem.h>

#include <Graphics/GRIS/RenderHelpers.h>

#include <ECS/SharedComponents/SceneComponent.h>

#include <ECS/EntityManager.h>

namespace CYD
{
uint32_t RenderSystem::getViewIndex( const SceneComponent& scene, std::string_view name ) const
{
   // Finding main view
   const auto& it = std::find( scene.viewNames.begin(), scene.viewNames.end(), name );
   if( it == scene.viewNames.end() )
   {
      // TODO WARNING
      CYD_ASSERT( !"Could not find main view, skipping render tick" );
      return 0;
   }

   return static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );
}

void RenderSystem::bindView(
    CmdListHandle cmdList,
    const SceneComponent& scene,
    const PipelineInfo* pipInfo,
    uint32_t viewIndex ) const
{
   CYD_ASSERT( pipInfo );

   const uint32_t viewOffset = viewIndex * sizeof( SceneComponent::ViewShaderParams );

   GRIS::NamedBufferBinding(
       cmdList,
       scene.viewsBuffer,
       "Views",
       *pipInfo,
       viewOffset,
       sizeof( SceneComponent::ViewShaderParams ) );
}

void RenderSystem::bindInverseView(
    CmdListHandle cmdList,
    const SceneComponent& scene,
    const PipelineInfo* pipInfo,
    uint32_t viewIndex ) const
{
   CYD_ASSERT( pipInfo );

   const uint32_t viewOffset = viewIndex * sizeof( SceneComponent::InverseViewShaderParams );

   GRIS::NamedBufferBinding(
       cmdList,
       scene.inverseViewsBuffer,
       "InverseViews",
       *pipInfo,
       viewOffset,
       sizeof( SceneComponent::InverseViewShaderParams ) );
}
}