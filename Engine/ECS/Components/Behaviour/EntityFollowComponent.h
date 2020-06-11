#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

// ================================================================================================
// Definition
// ================================================================================================
/*
This class is used to make an entity follow another entity. More specifically, the entity following
another entity will have the target entity's position.
*/
namespace CYD
{
class EntityFollowComponent : public BaseComponent
{
  public:
   EntityFollowComponent() = default;
   MOVABLE( EntityFollowComponent )
   virtual ~EntityFollowComponent() = default;

   bool init( EntityHandle followedEntity )
   {
      entity = followedEntity;
      return true;
   }
   void uninit() override {}

   static constexpr ComponentType TYPE = ComponentType::ENTITY_FOLLOW;

   EntityHandle entity = 0;
};
}