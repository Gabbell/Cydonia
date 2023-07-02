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

   RenderableComponent() = default;
   RenderableComponent( Type type ) : type( type ) {}
   COPIABLE( RenderableComponent );
   virtual ~RenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   Type type = Type::FORWARD;

   BufferHandle instancesBuffer;
   uint32_t instanceCount = 0;

   bool isVisible       = true;
   bool isShadowCasting = true;
   bool isInstanced     = false;
   bool isTransparent   = false;
};
}
