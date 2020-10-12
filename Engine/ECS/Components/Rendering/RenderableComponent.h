#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/StaticPipelines.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class RenderableComponent : public BaseComponent
{
  public:
   RenderableComponent() = default;
   RenderableComponent( StaticPipelines::Type pipType, std::string_view assetName )
       : type( pipType ), asset( assetName )
   {
   }
   COPIABLE( RenderableComponent );
   virtual ~RenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   // Used to determine which pipeline to use to render this entity and how to interpret the shader
   // resources attached to this renderable
   StaticPipelines::Type type = StaticPipelines::Type::DEFAULT;

   // Name of the material asset
   std::string_view asset;

   bool isOccluder = false;  // Should this renderable cast a shadow?
};
}
