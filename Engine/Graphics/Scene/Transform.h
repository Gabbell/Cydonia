#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace cyd
{
struct Transform final
{
   glm::vec3 pos     = glm::vec3( 0.0f );
   glm::vec3 scaling = glm::vec3( 1.0f );

   glm::quat rotation = glm::identity<glm::quat>();

   Transform() = default;
   Transform( const glm::vec3& origin );
   ~Transform() = default;

   glm::mat4 getModelMatrix() const;

   void translate( float x, float y, float z ) { pos += glm::vec3( x, y, z ); }
   void translate( const glm::vec3& translation ) { pos += translation; }
   void translateLocal( float x, float y, float z );
   void translateLocal( const glm::vec3 translation );

   void rotate( float pitch, float yaw, float roll );
   void rotateLocal( float pitch, float yaw, float roll );

   void scale( const glm::vec3& scale );
};
}
