#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Rendering/StaticMaterialComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;

class MaterialLoaderSystem final : public CommonSystem<StaticMaterialComponent>
{
  public:
   MaterialLoaderSystem( MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( MaterialLoaderSystem );
   virtual ~MaterialLoaderSystem() = default;

   void tick( double deltaS ) override;

  private:
   MaterialCache& m_materials;
};
}
