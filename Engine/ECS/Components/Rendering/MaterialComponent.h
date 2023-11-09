#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/GraphicsTypes.h>

#include <string_view>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class MaterialComponent final : public BaseComponent
{
  public:
   MaterialComponent() = default;
   MaterialComponent( std::string_view materialName ) : materialName( materialName ) {}
   COPIABLE( MaterialComponent );
   virtual ~MaterialComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MATERIAL;

   std::string_view materialName;

   MaterialIndex materialIdx = INVALID_MATERIAL_IDX;
};
}
