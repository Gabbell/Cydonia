#include <Core/Graphics/Camera.h>

static constexpr glm::vec4 DEFAULT_ORIGIN = glm::vec4( 0.0f, 0.0f, 0.0f, 0.0f );
static constexpr float DEFAULT_FOV        = 60.0f;
static constexpr float DEFAULT_NEAR       = 0.1f;
static constexpr float DEFAULT_FAR        = 1000.0f;
static constexpr float DEFAULT_RATIO      = 16.0f / 9.0f;  // 16:9

cyd::Camera::Camera()
    : _position( DEFAULT_ORIGIN ),
      _fov( DEFAULT_FOV ),
      _aspectRatio( DEFAULT_RATIO ),
      _near( DEFAULT_NEAR ),
      _far( DEFAULT_FAR )
{
}

cyd::Camera::Camera(
    const glm::vec4& origin,
    float fov,
    float aspectRatio,
    float nearPlane,
    float farPlane )
    : _position( origin ),
      _fov( fov ),
      _aspectRatio( aspectRatio ),
      _near( nearPlane ),
      _far( farPlane )
{
}
