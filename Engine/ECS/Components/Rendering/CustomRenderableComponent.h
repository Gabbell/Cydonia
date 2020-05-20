#pragma once

#include <ECS/Components/Rendering/RenderableComponent.h>

#include <string>

namespace CYD
{
class CustomRenderableComponent final : public RenderableComponent
{
  public:
   CustomRenderableComponent() = default;
   CustomRenderableComponent( std::string fragShader )
       : RenderableComponent( RenderableType::CUSTOM ), fragShader( std::move( fragShader ) )
   {
   }
   MOVABLE( CustomRenderableComponent )
   virtual ~CustomRenderableComponent();

   std::string fragShader;
};
}
