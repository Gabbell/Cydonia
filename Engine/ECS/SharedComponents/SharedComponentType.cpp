#include <ECS/SharedComponents/SharedComponentType.h>

namespace CYD
{
std::string_view GetSharedComponentName( SharedComponentType type )
{
   constexpr std::string_view SHARED_COMPONENT_NAMES[] = { "Input", "Scene", "Options" };

   static_assert(
       ARRSIZE( SHARED_COMPONENT_NAMES ) == static_cast<size_t>( SharedComponentType::COUNT ) );

   return SHARED_COMPONENT_NAMES[static_cast<size_t>( type )];
}
}