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
class EntityFollowComponent final : public BaseComponent
{
  public:
   EntityFollowComponent() = default;
   EntityFollowComponent( EntityHandle followedEntity ) : entity( followedEntity ) {}
   MOVABLE( EntityFollowComponent );
   virtual ~EntityFollowComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::ENTITY_FOLLOW;

   EntityHandle entity = 0;
};
}