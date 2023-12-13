#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/TessellatedComponent.h>

namespace CYD
{
class TessellationUpdateSystem final
    : public CommonSystem<RenderableComponent, TessellatedComponent>
{
  public:
   TessellationUpdateSystem() = default;
   NON_COPIABLE( TessellationUpdateSystem );
   virtual ~TessellationUpdateSystem() = default;

   void tick( double deltaS ) override;
};
}
