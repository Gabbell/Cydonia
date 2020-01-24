#include <ECS/ECS.h>

#include <ECS/SharedComponents/InputComponent.h>

namespace cyd::ECS
{
bool Initialize()
{
   // Initializing shared components
   detail::sharedComponents[size_t( SharedComponentType::INPUT )] = new InputComponent();

   return true;
}

void Tick( double deltaS )
{
   // Ordered updating
   for( auto& system : detail::systems )
   {
      if( system->hasToTick() )  // Must have to not needlessly tick the systems
      {
         system->tick( deltaS );
      }
   }
}

EntityHandle CreateEntity()
{
   // Building entity
   static int uid            = 0;
   const EntityHandle handle = uid++;
   detail::entities[handle]  = Entity( handle );

   return handle;
}

void RemoveEntity( EntityHandle /*handle*/ )
{
   //
}

void Uninitialize()
{
   for( auto& sharedComponent : detail::sharedComponents )
   {
      delete sharedComponent;
   }
   for( auto& componentPool : detail::components )
   {
      delete componentPool;
   }
   for( auto& system : detail::systems )
   {
      system->uninit();
      delete system;
   }
}
}
