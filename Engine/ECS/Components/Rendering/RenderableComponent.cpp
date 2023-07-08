#include <ECS/Components/Rendering/RenderableComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
RenderableComponent::~RenderableComponent()
{
   if( instancesBuffer )
   {
      GRIS::DestroyBuffer( instancesBuffer );
   }

   if( tessellationBuffer )
   {
      GRIS::DestroyBuffer( tessellationBuffer );
   }
}
}