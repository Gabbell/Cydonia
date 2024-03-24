#include <ECS/SharedComponents/SceneComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/Components/Debug/DebugDrawComponent.h>

namespace CYD
{
SceneComponent::SceneComponent()
{
   // Lights
   lightsBuffer = GRIS::CreateUniformBuffer( sizeof( lights ), "Scene Lights Buffer" );

   // Views
   viewsBuffer = GRIS::CreateUniformBuffer( sizeof( views ), "Scene Views Buffer" );
   inverseViewsBuffer =
       GRIS::CreateUniformBuffer( sizeof( inverseViews ), "Scene InverseViews Buffer" );
   frustumsBuffer   = GRIS::CreateUniformBuffer( sizeof( frustums ), "Scene Frustums Buffer" );
   shadowMapsBuffer = GRIS::CreateUniformBuffer( sizeof( shadowMaps ), "Scene Shadow Maps Buffer" );

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
   GRIS::DestroyBuffer( frustumsBuffer );
   GRIS::DestroyBuffer( shadowMapsBuffer );
   GRIS::DestroyTexture( envMap );
   GRIS::DestroyTexture( quarterResShadowMask );
   GRIS::DestroyTexture( mainColor );
   GRIS::DestroyTexture( mainDepth );
}
}