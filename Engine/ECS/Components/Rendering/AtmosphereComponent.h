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
class AtmosphereComponent final : public BaseComponent
{
  public:
   AtmosphereComponent() = default;
   COPIABLE( AtmosphereComponent );
   virtual ~AtmosphereComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::ATMOSPHERE;
};
}
