#include <ECS/Components/Rendering/RenderableComponent.h>

#include <Graphics/RenderInterface.h>

namespace CYD
{
bool RenderableComponent::init() { return true; }

void RenderableComponent::uninit() { GRIS::DestroyTexture( displacement ); }

RenderableComponent::~RenderableComponent() { RenderableComponent::uninit(); }
}
