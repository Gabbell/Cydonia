#pragma once

#include <Common/Include.h>

#include <tuple>

// ================================================================================================
// Definition
// ================================================================================================
/*
 * Systems are defined by archetypes. This means that there is one or more system PER archetype.
 */
namespace cyd
{
struct BaseArchetype  // This class should never be inherited anywhere else
{
  protected:
   BaseArchetype() = default;
};

template <class... ComponentTypes>
class Archetype final : public BaseArchetype
{
  public:
   Archetype() = default;
   COPIABLE( Archetype );
   ~Archetype() = default;

  private:
   // The component types this archetype is associated with.
   std::tuple<ComponentTypes...> types;
};
}
