#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class InstancedComponent final : public BaseComponent
{
  public:
   struct ShaderParams  // GPU
   {
      glm::mat4x4 modelMat;
   };

   InstancedComponent() = default;
   InstancedComponent( uint32_t count ) : count( count ) {}
   COPIABLE( InstancedComponent );
   virtual ~InstancedComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::INSTANCED;

   static constexpr uint32_t MAX_INSTANCES = 2048;

   uint32_t count   = 0;
   bool needsUpdate = true;
};
}
