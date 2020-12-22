#include <ECS/SharedComponents/SceneComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
SceneComponent::SceneComponent()
{
   lightsBuffer = GRIS::CreateUniformBuffer( sizeof( DirectionalLightUBO ) );
}

SceneComponent::~SceneComponent() { GRIS::DestroyBuffer( lightsBuffer ); }
}