#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Utility/ShaderMemoryLayout.h>

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
   enum class Type
   {
      TILED
   };

   struct ShaderParams  // GPU
   {
      uint32_t index;
      CYD_SHADER_ALIGN glm::mat4x4 modelMat;
   };

   InstancedComponent() = default;
   InstancedComponent( Type type, uint32_t count, float radius )
       : type( type ), count( count ), radius( radius )
   {
   }
   COPIABLE( InstancedComponent );
   virtual ~InstancedComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::INSTANCED;

   static constexpr uint32_t MAX_INSTANCES = 1024;

   Type type = Type::TILED;

   uint32_t count = 0;
   float radius   = 0;

   bool needsUpdate = true;
};
}
