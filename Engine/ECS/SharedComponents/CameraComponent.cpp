#include <ECS/SharedComponents/CameraComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
CameraComponent::CameraComponent()
{
   viewBuffer =
       GRIS::CreateUniformBuffer( sizeof( EnvironmentView ), "CameraComponent View Buffer" );
}

CameraComponent::~CameraComponent() { GRIS::DestroyBuffer( viewBuffer ); }
}