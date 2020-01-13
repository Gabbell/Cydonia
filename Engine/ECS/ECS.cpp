#include <ECS/ECS.h>

namespace cyd::ECS
{
bool Initialize() { return true; }

void Tick( double deltaMs )
{
   for( auto& system : detail::systems )
   {
      system->tick( deltaMs );
   }
}

EntityHandle CreateEntity()
{
   // Building entity
   static int uid            = 0;
   const EntityHandle handle = uid++;
   detail::entities[handle]  = {};

   return handle;
}

void RemoveEntity( EntityHandle handle )
{
   //
}

void Uninitialize()
{
   for( auto& component : detail::components )
   {
      delete component;
   }
   for( auto& system : detail::systems )
   {
      delete system;
   }
}
}
