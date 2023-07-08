#include <ECS/Components/Procedural/FogComponent.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
FogComponent::~FogComponent() { GRIS::DestroyBuffer( viewInfoBuffer ); }
}