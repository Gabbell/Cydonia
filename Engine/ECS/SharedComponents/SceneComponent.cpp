#include <ECS/SharedComponents/SceneComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/Components/Debug/DebugDrawComponent.h>

namespace CYD
{
SceneComponent::SceneComponent()
{
   lightsBuffer = GRIS::CreateUniformBuffer( sizeof( lights ), "SceneComponent Lights Buffer" );
   viewsBuffer  = GRIS::CreateUniformBuffer( sizeof( views ), "SceneComponent Views Buffer" );

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
}
}