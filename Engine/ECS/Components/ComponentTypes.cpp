#include <ECS/Components/ComponentTypes.h>

namespace CYD
{
std::string_view GetComponentName( ComponentType type )
{
   constexpr std::string_view COMPONENT_NAMES[] = {
       "Transform",
       "View",
       "Light",
       "Shadow Map",
       "Material",
       "Mesh",
       "Renderable",
       "Post-Process Volume",
       "Particles Volume",
       "Instanced",
       "Tessellated",
       "Displacement",
       "Ocean",
       "Fog",
       "Atmosphere",
       "Motion",
       "Entity Follow",
       "Debug Draw" };

   static_assert( ARRSIZE( COMPONENT_NAMES ) == static_cast<uint32_t>( ComponentType::COUNT ) );

   return COMPONENT_NAMES[static_cast<uint32_t>( type )];
}
}