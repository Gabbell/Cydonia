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
class ForwardRenderableComponent final : public BaseComponent
{
  public:
   ForwardRenderableComponent( bool castsShadows = false, bool visible = true )
       : castsShadows( castsShadows ), isVisible( visible )
   {
   }
   COPIABLE( ForwardRenderableComponent );
   virtual ~ForwardRenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::FORWARD_RENDERABLE;

   bool isTransparent : 1 = false;
   bool castsShadows : 1  = true;
   bool isVisible : 1     = true;
};
}
