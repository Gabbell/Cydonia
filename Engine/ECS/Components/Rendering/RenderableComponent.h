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
   RenderableComponent( bool castsShadows = false, bool visible = true )
       : castsShadows( castsShadows ), isVisible( visible )
   {
   }
   COPIABLE( RenderableComponent );
   virtual ~RenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   bool castsShadows : 1 = true;
   bool isVisible : 1    = true;
};
}
