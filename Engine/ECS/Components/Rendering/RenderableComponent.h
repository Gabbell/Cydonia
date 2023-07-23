#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Graphics/Handles/ResourceHandle.h>

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
   enum class Type
   {
      FORWARD,
      DEFERRED
   };

   RenderableComponent( bool isShadowCasting = false, bool isShadowReceiving = false )
       : isShadowCasting( isShadowCasting ), isShadowReceiving( isShadowReceiving )
   {
   }
   RenderableComponent( Type type ) : type( type ) {}
   COPIABLE( RenderableComponent );
   virtual ~RenderableComponent();

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   Type type = Type::FORWARD;

   BufferHandle tessellationBuffer;

   BufferHandle instancesBuffer;
   uint32_t instanceCount = 0;

   bool isVisible         = true;
   bool isShadowCasting   = false;
   bool isShadowReceiving = false;
   bool isInstanced       = false;
   bool isTessellated     = false;
   bool isTransparent     = false;
};
}
