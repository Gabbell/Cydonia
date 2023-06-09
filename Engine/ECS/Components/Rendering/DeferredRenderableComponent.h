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
class DeferredRenderableComponent final : public BaseComponent
{
  public:
   DeferredRenderableComponent( bool castsShadows = false, bool visible = true )
       : castsShadows( castsShadows ), isVisible( visible )
   {
   }
   COPIABLE( DeferredRenderableComponent );
   virtual ~DeferredRenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::DEFERRED_RENDERABLE;

   bool isTransparent : 1 = false;
   bool castsShadows : 1  = true;
   bool isVisible : 1     = true;
};
}
