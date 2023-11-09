#pragma once

#include <ECS/Systems/Rendering/RenderSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>

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