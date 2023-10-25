#include <ECS/SharedComponents/SceneComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/Components/Debug/DebugDrawComponent.h>

namespace CYD
{
SceneComponent::SceneComponent()
{
   // Lights
   lightsBuffer = GRIS::CreateUniformBuffer( sizeof( lights ), "SceneComponent Lights Buffer" );

   // Views
   viewsBuffer = GRIS::CreateUniformBuffer( sizeof( views ), "SceneComponent Views Buffer" );
   inverseViewsBuffer =
       GRIS::CreateUniformBuffer( sizeof( inverseViews ), "SceneComponent InverseViews Buffer" );

#if CYD_DEBUG
   debugParamsBuffer = GRIS::CreateUniformBuffer(
       sizeof( DebugDrawComponent::ShaderParams ), "Debug Params Buffer" );
#endif
}

SceneComponent::~SceneComponent()
{
#if CYD_DEBUG
   GRIS::DestroyBuffer( debugParamsBuffer );
#endif

   GRIS::DestroyBuffer( lightsBuffer );
   GRIS::DestroyBuffer( viewsBuffer );
   GRIS::DestroyBuffer( inverseViewsBuffer );
   GRIS::DestroyTexture( shadowMap );
   GRIS::DestroyTexture( mainColor );
   GRIS::DestroyTexture( mainDepth );
}
}