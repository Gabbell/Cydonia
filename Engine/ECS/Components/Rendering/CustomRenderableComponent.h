#pragma once

#include <ECS/Components/Rendering/RenderableComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <string>
#include <vector>

namespace CYD
{
class CustomRenderableComponent final : public RenderableComponent
{
  public:
   CustomRenderableComponent() = default;
   CustomRenderableComponent( const std::string vertShader, const std::string fragShader )
       : vertShader( std::move( vertShader ) ), fragShader( std::move( fragShader ) )
   {
   }
   MOVABLE( CustomRenderableComponent )
   virtual ~CustomRenderableComponent();

   static constexpr ComponentType SUBTYPE = ComponentType::RENDERABLE_CUSTOM;

   std::string vertShader;
   std::string fragShader;
};
}
