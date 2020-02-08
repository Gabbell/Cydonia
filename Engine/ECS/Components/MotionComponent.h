#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <glm/glm.hpp>

namespace cyd
{
class MotionComponent final : public BaseComponent
{
  public:
   MotionComponent() = default;
   MotionComponent( const glm::vec3& aVelocity, const glm::vec3& aAcceleration )
       : velocity( aVelocity ), acceleration( aAcceleration )
   {
   }
   COPIABLE( MotionComponent );
   virtual ~MotionComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MOTION;

   bool init() override { return true; }
   void uninit() override {}

   glm::vec3 velocity     = glm::vec3( 0.0f );
   glm::vec3 acceleration = glm::vec3( 0.0f );
};
}
