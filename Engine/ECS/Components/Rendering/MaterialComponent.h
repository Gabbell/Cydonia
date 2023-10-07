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
   struct Description
   {
      std::string pipelineName;
      std::string materialName;
   } description;

   MaterialComponent() = default;
   MaterialComponent( const Description& desc ) : description( desc ) {}
   COPIABLE( MaterialComponent );
   virtual ~MaterialComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MATERIAL;

   PipelineIndex pipelineIdx = INVALID_PIPELINE_IDX;
   MaterialIndex materialIdx = INVALID_MATERIAL_IDX;

   bool isLoaded = false;
};
}
