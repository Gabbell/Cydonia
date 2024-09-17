#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
// Raymarches shadow rays. This is pretty specialized to terrain rendering right now and uses
// the terrain heightmap to raymarch.
namespace CYD
{
class MaterialCache;

class ShadowVolumeSystem final
    : public CommonSystem<RenderableComponent, TransformComponent, MaterialComponent>
{
  public:
   ShadowVolumeSystem() = delete;
   ShadowVolumeSystem( const MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( ShadowVolumeSystem );
   virtual ~ShadowVolumeSystem() = default;

   void tick( double deltaS ) override;

  private:
   const MaterialCache& m_materials;
};
}
