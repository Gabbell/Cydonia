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
class FullscreenComponent final : public BaseComponent
{
  public:
   FullscreenComponent() = default;
   COPIABLE( FullscreenComponent );
   virtual ~FullscreenComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::FULLSCREEN;
};
}
