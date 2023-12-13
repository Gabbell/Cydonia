#include <ECS/Components/Rendering/RenderableComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
RenderableComponent::~RenderableComponent()
{
   GRIS::DestroyBuffer( instancesBuffer );
   GRIS::DestroyBuffer( tessellationBuffer );
}
}