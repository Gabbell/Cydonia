#include <ECS/Components/Procedural/NoiseComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
NoiseComponent::~NoiseComponent()
{
   GRIS::DestroyTexture( texture );
}
}
