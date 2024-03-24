#pragma once

#include <ECS/Systems/Rendering/RenderSystem.h>

#include <Common/Include.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class ShadowMapSystem final : public RenderSystem
{
  public:
   ShadowMapSystem() = delete;
   ShadowMapSystem( const MeshCache& meshes, const MaterialCache& materials )
       : RenderSystem( meshes, materials )
   {
   }
   NON_COPIABLE( ShadowMapSystem );
   virtual ~ShadowMapSystem() = default;

   void tick( double deltaS ) override;
};
}