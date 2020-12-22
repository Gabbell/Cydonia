#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/GraphicsTypes.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class RenderableComponent final : public BaseComponent
{
  public:
   RenderableComponent() = default;
   RenderableComponent( bool occluder, bool visible = true )
       : isVisible( visible ), isOccluder( occluder )
   {
   }
   COPIABLE( RenderableComponent );
   virtual ~RenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   bool isVisible  = true;
   bool isOccluder = false;
};
}
