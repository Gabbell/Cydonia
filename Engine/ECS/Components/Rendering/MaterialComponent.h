#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Scene/Material.h>

#include <Graphics/StaticPipelines.h>

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
   MaterialComponent( std::string_view pipelineName, std::string_view materialName )
       : pipelineName( pipelineName ), materialName( materialName )
   {
   }
   COPIABLE( MaterialComponent );
   virtual ~MaterialComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MATERIAL;

   std::string pipelineName;
   std::string materialName;

   PipelineIndex pipelineIdx = INVALID_PIPELINE_IDX;
   MaterialIndex materialIdx = INVALID_MATERIAL_IDX;

   bool isLoaded = false;
};
}
