#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class ImGuiSystem final : public CommonSystem<>
{
  public:
   ImGuiSystem() = delete;
   ImGuiSystem( const EntityManager& entityManager );
   NON_COPIABLE( ImGuiSystem );
   virtual ~ImGuiSystem();

   void tick( double deltaS ) override;

  private:
   const EntityManager& m_entityManager;
};
}
