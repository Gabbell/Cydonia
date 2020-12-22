#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/MaterialComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class AssetStash;

class MaterialLoaderSystem final : public CommonSystem<MaterialComponent>
{
  public:
   MaterialLoaderSystem( AssetStash& assets ) : m_assets( assets ) {}
   NON_COPIABLE( MaterialLoaderSystem );
   virtual ~MaterialLoaderSystem() = default;

   void tick( double deltaS ) override;

  private:
   AssetStash& m_assets;
};
}
