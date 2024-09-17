#include <Graphics/Utility/Transforms.h>

#include <glm/gtc/reciprocal.hpp>

namespace CYD::Transform
{
void Rotate( glm::quat& rotation, float pitch, float yaw, float roll )
{
   const glm::quat rotPitch = glm::angleAxis( glm::radians( pitch ), glm::vec3( 1, 0, 0 ) );
   const glm::quat rotYaw   = glm::angleAxis( glm::radians( yaw ), glm::vec3( 0, 1, 0 ) );
   const glm::quat rotRoll  = glm::angleAxis( glm::radians( roll ), glm::vec3( 0, 0, 1 ) );

   rotation = rotRoll * rotPitch * rotYaw * rotation;
}

void RotateLocal( glm::quat& rotation, float pitch, float yaw, float roll )
{
   const glm::vec3 localXAxis = glm::rotate( rotation, glm::vec3( 1, 0, 0 ) );
   const glm::vec3 localYAxis = glm::rotate( rotation, glm::vec3( 0, 1, 0 ) );
   const glm::vec3 localZAxis = glm::rotate( rotation, glm::vec3( 0, 0, 1 ) );

   const glm::quat rotPitch = glm::angleAxis( glm::radians( pitch ), localXAxis );
   const glm::quat rotYaw   = glm::angleAxis( glm::radians( yaw ), localYAxis );
   const glm::quat rotRoll  = glm::angleAxis( glm::radians( roll ), localZAxis );

   rotation = rotRoll * rotPitch * rotYaw * rotation;
}

void Translate( glm::vec3& position, float x, float y, float z )
{
   position += glm::vec3( x, y, z );
}

void Translate( glm::vec3& position, const glm::vec3& translation ) { position += translation; }

void TranslateLocal( glm::vec3& position, const glm::quat& rotation, float x, float y, float z )
{
   Translate( position, glm::rotate( rotation, glm::vec3( x, y, z ) ) );
}

void TranslateLocal( glm::vec3& position, const glm::quat& rotation, const glm::vec3& translation )
{
   Translate( position, glm::rotate( rotation, translation ) );
}

glm::mat4
GetModelMatrix( const glm::vec3& scaling, const glm::quat& rotation, const glm::vec3& position )
{
   glm::mat4 translate = glm::translate( glm::mat4( 1.0f ), position );
   glm::mat4 rotate    = glm::toMat4( glm::conjugate( rotation ) );
   glm::mat4 scale     = glm::scale( glm::mat4( 1.0f ), scaling );
   return rotate * translate * scale;
}

glm::mat4 Ortho(
    float left,
    float right,
    float bottom,
    float top,
    float near,
    float far,
    bool invertY,
    bool reverseZ )
{
   if( reverseZ )
   {
      // Invert near and far planes
      std::swap( near, far );
   }

   glm::mat4 proj = glm::orthoLH_ZO( left, right, bottom, top, near, far );

   if( invertY )
   {
      proj[1][1] *= -1.0f;  // Invert Y
   }

   return proj;
}

glm::mat4 Perspective(
    float fov,
    float width,
    float height,
    float near,
    float far,
    bool invertY,
    bool reverseZ )
{
   if( reverseZ )
   {
      // Invert near and far planes
      std::swap( near, far );
   }

   glm::mat4 proj = glm::perspectiveFovLH_ZO( glm::radians( fov ), width, height, near, far );

   if( invertY )
   {
      proj[1][1] *= -1.0f;  // Invert Y
   }

   return proj;
}
}
