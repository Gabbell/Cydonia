#include <ECS/Systems/PlayerInputSystem.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace cyd
{
bool PlayerInputSystem::init() { return true; }

static void translate( TransformComponent* transform, const glm::vec3& translation )
{
   transform->position += translation;
}

static void translateLocal( TransformComponent* transform, glm::vec3 translation )
{
   const glm::vec3 delta = glm::rotate( transform->rotation, translation );
   translate( transform, delta );
}

static void rotate( TransformComponent* transform, float pitch, float yaw, float roll )
{
   const glm::quat rotPitch = glm::angleAxis( pitch, glm::vec3( 1, 0, 0 ) );
   const glm::quat rotYaw   = glm::angleAxis( yaw, glm::vec3( 0, 1, 0 ) );
   const glm::quat rotRoll  = glm::angleAxis( roll, glm::vec3( 0, 0, 1 ) );
   transform->rotation      = rotRoll * rotPitch * rotYaw * transform->rotation;
}

static void rotateLocal( TransformComponent* transform, float pitch, float yaw, float roll )
{
   const glm::vec3 localXAxis = glm::rotate( transform->rotation, glm::vec3( 1, 0, 0 ) );
   const glm::vec3 localYAxis = glm::rotate( transform->rotation, glm::vec3( 0, 1, 0 ) );
   const glm::vec3 localZAxis = glm::rotate( transform->rotation, glm::vec3( 0, 0, 1 ) );

   const glm::quat rotPitch = glm::angleAxis( pitch, localXAxis );
   const glm::quat rotYaw   = glm::angleAxis( yaw, localYAxis );
   const glm::quat rotRoll  = glm::angleAxis( roll, localZAxis );
   transform->rotation      = rotRoll * rotPitch * rotYaw * transform->rotation;
}

void PlayerInputSystem::tick( double deltaMs )
{
   for( const auto& tuple : m_archetypes )
   {
      TransformComponent* transform       = std::get<TransformComponent*>( tuple );
      FreeControllerComponent* controller = std::get<FreeControllerComponent*>( tuple );

      glm::vec3 displacement( 0.0f );
      if( controller->goingForwards )
      {
         displacement.z -= FreeControllerComponent::MOVE_SPEED;
      }
      if( controller->goingBackwards )
      {
         displacement.z += FreeControllerComponent::MOVE_SPEED;
      }
      if( controller->goingRight )
      {
         displacement.x += FreeControllerComponent::MOVE_SPEED;
      }
      if( controller->goingLeft )
      {
         displacement.x -= FreeControllerComponent::MOVE_SPEED;
      }

      if( displacement != glm::vec3( 0.0f ) )
      {
         translateLocal( transform, displacement );
      }

      if( controller->rotating )
      {
         const glm::vec2 rotationAngles =
             controller->cursorDelta * FreeControllerComponent::MOUSE_SENS;

         rotateLocal( transform, -rotationAngles.y, 0, 0 );
         rotate( transform, 0, rotationAngles.x, 0 );

         controller->cursorDelta = glm::vec2( 0.0f );
      }
   }
}
}
