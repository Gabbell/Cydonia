#pragma once

#include <ECS/SharedComponents/BaseSharedComponent.h>

#include <glm/glm.hpp>

namespace CYD
{
class CameraComponent final : public BaseSharedComponent
{
  public:
   CameraComponent() = default;
   NON_COPIABLE( CameraComponent );
   virtual ~CameraComponent() = default;

   static constexpr SharedComponentType TYPE = SharedComponentType::CAMERA;

   struct ViewProjection
   {
      glm::mat4 view = glm::mat4( 1.0f );
      glm::mat4 proj = glm::mat4( 1.0f );
   } vp;

   glm::vec4 pos;
};
}
