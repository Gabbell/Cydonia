#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace CYD::Transform
{
void Rotate( glm::quat& rotation, float pitch, float yaw, float roll );
void RotateLocal( glm::quat& rotation, float pitch, float yaw, float roll );
void Translate( glm::vec3& position, float x, float y, float z );
void Translate( glm::vec3& position, const glm::vec3& translation );
void TranslateLocal( glm::vec3& position, const glm::quat& rotation, float x, float y, float z );
void TranslateLocal( glm::vec3& position, const glm::quat& rotation, const glm::vec3& translation );
}
