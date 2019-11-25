#include <Core/Graphics/Scene/Camera.h>

#include <Core/Common/Assert.h>

#include <Core/Graphics/Scene/Transform.h>

#include <glm/gtx/transform.hpp>

static constexpr glm::vec3 DEFAULT_ORIGIN = glm::vec3( 0.0f, 0.0f, 0.0f );
static constexpr glm::vec3 DEFAULT_UP     = glm::vec3( 0.0f, -1.0f, 0.0f );
static constexpr float DEFAULT_FOV        = 60.0f;
static constexpr float DEFAULT_NEAR       = 0.001f;
static constexpr float DEFAULT_FAR        = 100.0f;
static constexpr float DEFAULT_RATIO      = 16.0f / 9.0f;  // 16:9

cyd::Camera::Camera( const Rectangle& viewport )
    : _viewport( viewport ),
      _fov( DEFAULT_FOV ),
      _aspectRatio( DEFAULT_RATIO ),
      _near( DEFAULT_NEAR ),
      _far( DEFAULT_FAR )
{
   transform = std::make_unique<Transform>( DEFAULT_ORIGIN );

   _view = glm::mat4( 1.0f );
   usePerspective();

   updateVP();
}

void cyd::Camera::updateVP()
{
   glm::mat4 t = glm::translate( glm::mat4( 1.0f ), -transform->pos );
   glm::mat4 s = glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) / transform->scaling );
   _view       = glm::toMat4( glm::conjugate( transform->rotation ) ) * s * t;
}

void cyd::Camera::usePerspective()
{
   if( _projMode == ProjectionMode::PERSPECTIVE )
   {
      CYDASSERT( !"Camera: Was already in perspective mode, no need to switch" );
      return;
   }

   _proj     = glm::perspectiveZO( glm::radians( _fov ), _aspectRatio, _near, _far );
   _projMode = ProjectionMode::PERSPECTIVE;
}

void cyd::Camera::useOrthographic()
{
   if( _projMode == ProjectionMode::ORTHOGRAPHIC )
   {
      CYDASSERT( !"Camera: Was already in orthographic mode, no need to switch" );
      return;
   }

   _proj     = glm::orthoZO( -1.0f, 1.0f, -1.0f, 1.0f, _near, _far );
   _projMode = ProjectionMode::PERSPECTIVE;
}

cyd::Camera::~Camera() {}
