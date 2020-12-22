#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/MeshComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class AssetStash;

class MeshLoaderSystem final : public CommonSystem<MeshComponent>
{
  public:
   MeshLoaderSystem( AssetStash& assets ) : m_assets( assets ) {}
   NON_COPIABLE( MeshLoaderSystem );
   virtual ~MeshLoaderSystem() = default;

   void tick( double deltaS ) override;

  private:
   AssetStash& m_assets;
};
}
