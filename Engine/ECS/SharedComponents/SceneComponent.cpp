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
   debugParamsBuffer =
       GRIS::CreateUniformBuffer( sizeof( DebugDrawComponent::ParamsUBO ), "Debug Params Buffer" );
#endif
}

SceneComponent::~SceneComponent()
{
   GRIS::DestroyBuffer( lightsBuffer );
   GRIS::DestroyBuffer( viewsBuffer );
}
}