#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace cyd
{
class TransformComponent final : public BaseComponent
{
  public:
   TransformComponent() = default;
   NON_COPIABLE( TransformComponent );
   virtual ~TransformComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::TRANSFORM;

   bool init( const void* pDescription ) override;

   glm::vec3 position = glm::vec3( 0.0f );
   glm::vec3 scaling  = glm::vec3( 1.0f );
   glm::quat rotation = glm::identity<glm::quat>();
};
}
