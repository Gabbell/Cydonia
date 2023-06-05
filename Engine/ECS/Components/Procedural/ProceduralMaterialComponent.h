#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

namespace CYD
{
class ProceduralMaterialComponent final : public BaseComponent
{
  public:
   ProceduralMaterialComponent() = default;
   COPIABLE( ProceduralMaterialComponent );
   virtual ~ProceduralMaterialComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::PROCEDURAL_MATERIAL;

   TextureHandle texture;
};
}
