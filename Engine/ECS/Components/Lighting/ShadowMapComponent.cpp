#include <ECS/Components/Lighting/ShadowMapComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
ShadowMapComponent::~ShadowMapComponent()
{
   GRIS::DestroyTexture( depth );
   GRIS::DestroyTexture( filterable );
}
}