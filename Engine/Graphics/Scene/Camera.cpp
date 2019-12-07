#include <Graphics/Scene/Camera.h>

#include <Common/Assert.h>

#include <glm/gtx/transform.hpp>

static constexpr glm::vec3 DEFAULT_ORIGIN = glm::vec3( 0.0f, 0.0f, 0.0f );
static constexpr float DEFAULT_FOV        = 60.0f;
static constexpr float DEFAULT_NEAR       = 0.001f;
static constexpr float DEFAULT_FAR        = 100.0f;
static constexpr float DEFAULT_RATIO      = 16.0f / 9.0f;  // 16:9

namespace cyd
{
// Perspective Constructors
// =================================================================================================
Camera::Camera() : Camera( DEFAULT_ORIGIN, DEFAULT_FOV, DEFAULT_RATIO, DEFAULT_NEAR, DEFAULT_FAR )
{
}

Camera::Camera( float fov, float aspectRatio, float near, float far )
    : Camera( DEFAULT_ORIGIN, fov, aspectRatio, near, far )
{
}

Camera::Camera( const glm::vec3& origin, float fov, float aspectRatio, float near, float far )
    : transform( origin ),
      _proj( glm::mat4( 1.0f ) ),
      _view( glm::mat4( 1.0f ) ),
      _fov( fov ),
      _aspectRatio( aspectRatio ),
      _near( near ),
      _far( far )
{
   usePerspective();
   updateVP();
}

// Orthographic Constructor
// =================================================================================================

Camera::Camera(
    const glm::vec3& origin,
    float left,
    float right,
    float bottom,
    float top,
    float near,
    float far )
    : transform( origin ),
      _proj( glm::mat4( 1.0f ) ),
      _view( glm::mat4( 1.0f ) ),
      _left( left ),
      _right( right ),
      _bottom( bottom ),
      _top( top ),
      _near( near ),
      _far( far )
{
   useOrthographic();
   updateVP();
}

void Camera::updateVP()
{
   glm::mat4 t = glm::translate( glm::mat4( 1.0f ), -transform.pos );
   glm::mat4 s = glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) / transform.scaling );
   _view       = glm::toMat4( glm::conjugate( transform.rotation ) ) * s * t;
}

void Camera::usePerspective()
{
   if( _projMode == ProjectionMode::PERSPECTIVE )
   {
      CYDASSERT( !"Camera: Was already in perspective mode, no need to switch" );
      return;
   }

   _proj     = glm::perspectiveZO( glm::radians( _fov ), _aspectRatio, _near, _far );
   _projMode = ProjectionMode::PERSPECTIVE;
}

void Camera::useOrthographic()
{
   if( _projMode == ProjectionMode::ORTHOGRAPHIC )
   {
      CYDASSERT( !"Camera: Was already in orthographic mode, no need to switch" );
      return;
   }

   _proj     = glm::orthoZO( _left, _right, _bottom, _top, _near, _far );
   _projMode = ProjectionMode::ORTHOGRAPHIC;
}

Camera::~Camera() {}
}