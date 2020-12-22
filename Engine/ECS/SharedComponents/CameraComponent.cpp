#include <ECS/SharedComponents/CameraComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
CameraComponent::CameraComponent()
{
   viewProjBuffer = GRIS::CreateUniformBuffer( sizeof( ViewProjection ) );
   positionBuffer = GRIS::CreateUniformBuffer( sizeof( glm::vec4 ) );
}

CameraComponent::~CameraComponent()
{
   GRIS::DestroyBuffer( positionBuffer );
   GRIS::DestroyBuffer( viewProjBuffer );
}
}