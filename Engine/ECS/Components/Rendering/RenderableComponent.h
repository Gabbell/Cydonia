#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Handles/Handle.h>

#include <ECS/Components/ComponentTypes.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/RenderableTypes.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This class is the base class for different types of renderables. You must inherit from this class
for the render system to be able to see the entity.
*/
namespace CYD
{
class RenderableComponent : public BaseComponent
{
  public:
   RenderableComponent() = default;
   MOVABLE( RenderableComponent )
   virtual ~RenderableComponent() = default;

   RenderableType getType() const noexcept { return type; }

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

  protected:
   explicit RenderableComponent( RenderableType aType ) : type( aType ) {}

   RenderableType type = RenderableType::DEFAULT;
};
}