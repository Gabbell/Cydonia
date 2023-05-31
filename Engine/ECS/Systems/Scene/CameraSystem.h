#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Scene/CameraComponent.h>

namespace CYD
{
class CameraSystem final : public CommonSystem<TransformComponent, CameraComponent>
{
  public:
   CameraSystem() = default;
   NON_COPIABLE( CameraSystem );
   virtual ~CameraSystem() = default;

   void tick( double deltaS ) override;
};
}
