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
class StaticMaterialComponent final : public BaseComponent
{
  public:
   StaticMaterialComponent() = default;
   StaticMaterialComponent( std::string_view pipelineName, std::string_view materialName )
       : pipelineName( pipelineName ), materialName( materialName )
   {
   }
   COPIABLE( StaticMaterialComponent );
   virtual ~StaticMaterialComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MATERIAL;

   std::string pipelineName;
   std::string materialName;

   PipelineIndex pipelineIdx = INVALID_PIPELINE_IDX;
   MaterialIndex materialIdx = INVALID_MATERIAL_IDX;

   bool isLoaded = false;
};
}
