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
    : transform( origin ), m_fov( fov ), m_aspectRatio( aspectRatio ), m_near( near ), m_far( far )
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
      m_near( near ),
      m_far( far ),
      m_left( left ),
      m_right( right ),
      m_bottom( bottom ),
      m_top( top )
{
   useOrthographic();
   updateVP();
}

void Camera::updateVP()
{
   glm::mat4 t = glm::translate( glm::mat4( 1.0f ), -transform.pos );
   glm::mat4 s = glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) / transform.scaling );
   m_view       = glm::toMat4( glm::conjugate( transform.rotation ) ) * s * t;
}

void Camera::usePerspective()
{
   if( m_projMode == ProjectionMode::PERSPECTIVE )
   {
      CYDASSERT( !"Camera: Was already in perspective mode, no need to switch" );
      return;
   }

   m_proj     = glm::perspectiveZO( glm::radians( m_fov ), m_aspectRatio, m_near, m_far );
   m_projMode = ProjectionMode::PERSPECTIVE;
}

void Camera::useOrthographic()
{
   if( m_projMode == ProjectionMode::ORTHOGRAPHIC )
   {
      CYDASSERT( !"Camera: Was already in orthographic mode, no need to switch" );
      return;
   }

   m_proj     = glm::orthoZO( m_left, m_right, m_bottom, m_top, m_near, m_far );
   m_projMode = ProjectionMode::ORTHOGRAPHIC;
}

Camera::~Camera() {}
}