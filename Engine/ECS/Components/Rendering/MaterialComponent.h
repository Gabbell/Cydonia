#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/Material.h>

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
   MaterialComponent( const std::string_view pipName, const std::string_view assetName = "" )
       : data{ pipName }, asset( assetName )
   {
   }
   COPIABLE( MaterialComponent );
   virtual ~MaterialComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MATERIAL;

   std::string_view asset;

   // Loaded dynamically
   Material data;
};
}
