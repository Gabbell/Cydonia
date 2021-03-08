#include <ECS/SharedComponents/SceneComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
SceneComponent::SceneComponent()
{
   lightsBuffer = GRIS::CreateUniformBuffer( sizeof( LightUBO ), "SceneComponent Lights Buffer" );
}

SceneComponent::~SceneComponent()
{
   GRIS::DestroyBuffer( lightsBuffer );
   GRIS::DestroyTexture( shadowMap );
}
}