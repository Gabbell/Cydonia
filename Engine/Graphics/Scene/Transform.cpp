#include <Graphics/Scene/Transform.h>

#include <glm/gtx/transform.hpp>

namespace cyd
{
Transform::Transform( const glm::vec3& origin ) : pos( origin ) {}

glm::mat4 Transform::getModelMatrix() const
{
   glm::mat4 translation = glm::translate( glm::mat4( 1.0f ), pos );
   glm::mat4 scale       = glm::scale( glm::mat4( 1.0f ), scaling );

   return translation * scale * glm::toMat4( rotation );
}

void Transform::translateLocal( float x, float y, float z )
{
   glm::vec3 delta = glm::rotate( rotation, glm::vec3( x, y, z ) );
   translate( delta );
}

void Transform::translateLocal( const glm::vec3 translation )
{
   glm::vec3 delta = glm::rotate( rotation, translation );
   translate( delta );
}

void Transform::rotate( float pitch, float yaw, float roll )
{
   glm::quat rotPitch = glm::angleAxis( pitch, glm::vec3( 1, 0, 0 ) );
   glm::quat rotYaw   = glm::angleAxis( yaw, glm::vec3( 0, 1, 0 ) );
   glm::quat rotRoll  = glm::angleAxis( roll, glm::vec3( 0, 0, 1 ) );
   rotation           = rotRoll * rotPitch * rotYaw * rotation;
}

void Transform::rotateLocal( float pitch, float yaw, float roll )
{
   glm::vec3 localXAxis = glm::rotate( rotation, glm::vec3( 1, 0, 0 ) );
   glm::vec3 localYAxis = glm::rotate( rotation, glm::vec3( 0, 1, 0 ) );
   glm::vec3 localZAxis = glm::rotate( rotation, glm::vec3( 0, 0, 1 ) );

   glm::quat rotPitch = glm::angleAxis( pitch, localXAxis );
   glm::quat rotYaw   = glm::angleAxis( yaw, localYAxis );
   glm::quat rotRoll  = glm::angleAxis( roll, localZAxis );
   rotation           = rotRoll * rotPitch * rotYaw * rotation;
}

void Transform::scale( const glm::vec3& scale ) { scaling *= scale; }
}