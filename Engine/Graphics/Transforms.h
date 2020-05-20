#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace CYD::Transform
{
void rotate( glm::quat& rotation, float pitch, float yaw, float roll );
void rotateLocal( glm::quat& rotation, float pitch, float yaw, float roll );
void translate( glm::vec3& position, float x, float y, float z );
void translate( glm::vec3& position, const glm::vec3& translation );
void translateLocal( glm::vec3& position, const glm::quat& rotation, float x, float y, float z );
void translateLocal( glm::vec3& position, const glm::quat& rotation, const glm::vec3& translation );
}
