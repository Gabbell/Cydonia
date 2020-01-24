#pragma once

#include <ECS/Components/ComponentTypes.h>

#include <bitset>

namespace cyd
{
class BaseComponent;
}

namespace cyd
{
class Archetype
{
  public:
   Archetype()  = default;  // Initialize bitset to 0
   ~Archetype() = default;

   bool operator==( const Archetype& other ) const { return m_flags == other.m_flags; }

   // Add a component to the archetype
   template <class Component>
   void update()
   {
      static_assert( std::is_base_of_v<BaseComponent, Component> );
      m_flags.set( Component::TYPE, true );
   }

   // Remove a component from the archetype
   template <class Component>
   void downgrade()
   {
      static_assert( std::is_base_of_v<BaseComponent, Component> );
      m_flags.set( Component::TYPE, false );
   }

  private:
   std::bitset<static_cast<size_t>( ComponentType::COUNT )> m_flags;
};
}
