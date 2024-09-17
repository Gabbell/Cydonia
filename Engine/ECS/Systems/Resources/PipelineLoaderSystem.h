#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/RenderableComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class PipelineLoaderSystem final : public CommonSystem<RenderableComponent>
{
  public:
   PipelineLoaderSystem() = default;
   NON_COPIABLE( PipelineLoaderSystem );
   virtual ~PipelineLoaderSystem() = default;

   void tick( double deltaS ) override;
};
}
