#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/StaticMaterialComponent.h>
#include <ECS/Components/Rendering/FullscreenComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class FullscreenRenderSystem final : public CommonSystem<StaticMaterialComponent, FullscreenComponent>
{
  public:
   FullscreenRenderSystem() = delete;
   FullscreenRenderSystem( const MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( FullscreenRenderSystem );
   virtual ~FullscreenRenderSystem() = default;

   void tick( double deltaS ) override;

  private:
   const MaterialCache& m_materials;
};
}
