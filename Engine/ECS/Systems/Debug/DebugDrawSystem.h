#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Debug/DebugDrawComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MeshCache;

class DebugDrawSystem final : public CommonSystem<TransformComponent, DebugDrawComponent>
{
  public:
   DebugDrawSystem( MeshCache& meshCache ) : m_meshes( meshCache ) {}
   NON_COPIABLE( DebugDrawSystem );
   virtual ~DebugDrawSystem() = default;

   void tick( double deltaS ) override;

  private:
   MeshCache& m_meshes;
};
}
