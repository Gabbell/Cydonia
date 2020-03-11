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
   TransformComponent( const glm::vec3& aPosition ) : position( aPosition ) {}
   TransformComponent(
       const glm::vec3& aPosition,
       const glm::vec3& aScaling,
       const glm::quat& aRotation )
       : position( aPosition ), scaling( aScaling ), rotation( aRotation )
   {
   }
   COPIABLE( TransformComponent );
   virtual ~TransformComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::TRANSFORM;

   bool init() override { return true; }
   void uninit() override{};

   glm::vec3 position = glm::vec3( 0.0f );
   glm::vec3 scaling  = glm::vec3( 1.0f );
   glm::quat rotation = glm::identity<glm::quat>();
};
}
