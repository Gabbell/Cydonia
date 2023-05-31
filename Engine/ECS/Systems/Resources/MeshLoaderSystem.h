#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/MeshComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MeshCache;

class MeshLoaderSystem final : public CommonSystem<MeshComponent>
{
  public:
   MeshLoaderSystem( MeshCache& meshCache ) : m_meshCache( meshCache ) {}
   NON_COPIABLE( MeshLoaderSystem );
   virtual ~MeshLoaderSystem() = default;

   void tick( double deltaS ) override;

  private:
   MeshCache& m_meshCache;
};
}
