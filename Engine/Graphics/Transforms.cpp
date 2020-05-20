#include <Graphics/Transforms.h>

namespace CYD::Transform
{
void rotate( glm::quat& rotation, float pitch, float yaw, float roll )
{
   const glm::quat rotPitch = glm::angleAxis( pitch, glm::vec3( 1, 0, 0 ) );
   const glm::quat rotYaw   = glm::angleAxis( yaw, glm::vec3( 0, 1, 0 ) );
   const glm::quat rotRoll  = glm::angleAxis( roll, glm::vec3( 0, 0, 1 ) );

   rotation = rotRoll * rotPitch * rotYaw * rotation;
}

void rotateLocal( glm::quat& rotation, float pitch, float yaw, float roll )
{
   const glm::vec3 localXAxis = glm::rotate( rotation, glm::vec3( 1, 0, 0 ) );
   const glm::vec3 localYAxis = glm::rotate( rotation, glm::vec3( 0, 1, 0 ) );
   const glm::vec3 localZAxis = glm::rotate( rotation, glm::vec3( 0, 0, 1 ) );

   const glm::quat rotPitch = glm::angleAxis( pitch, localXAxis );
   const glm::quat rotYaw   = glm::angleAxis( yaw, localYAxis );
   const glm::quat rotRoll  = glm::angleAxis( roll, localZAxis );

   rotation = rotRoll * rotPitch * rotYaw * rotation;
}

void translate( glm::vec3& position, float x, float y, float z )
{
   position += glm::vec3( x, y, z );
}

void translate( glm::vec3& position, const glm::vec3& translation ) { position += translation; }

void translateLocal( glm::vec3& position, const glm::quat& rotation, float x, float y, float z )
{
   translate( position, glm::rotate( rotation, glm::vec3( x, y, z ) ) );
}

void translateLocal( glm::vec3& position, const glm::quat& rotation, const glm::vec3& translation )
{
   translate( position, glm::rotate( rotation, translation ) );
}
}
