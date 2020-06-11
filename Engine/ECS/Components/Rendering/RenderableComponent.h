#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Handles/Handle.h>

#include <ECS/Components/ComponentTypes.h>

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
   virtual ~RenderableComponent();

   bool init();
   void uninit() override;

   virtual ComponentType getType() const { return TYPE; }

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   TextureHandle displacement;
};
}