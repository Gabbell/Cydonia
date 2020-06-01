#pragma once

#include <ECS/Components/Rendering/RenderableComponent.h>

#include <string>
#include <vector>

namespace CYD
{
class CustomRenderableComponent final : public RenderableComponent
{
  public:
   CustomRenderableComponent() = default;
   CustomRenderableComponent( const std::vector<std::string>& shaders )
       : RenderableComponent( RenderableType::CUSTOM ), shaders( std::move( shaders ) )
   {
   }
   MOVABLE( CustomRenderableComponent )
   virtual ~CustomRenderableComponent();

   std::vector<std::string> shaders;
};
}
