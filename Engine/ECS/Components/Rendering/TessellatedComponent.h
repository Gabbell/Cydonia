#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <glm/glm.hpp>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class TessellatedComponent final : public BaseComponent
{
  public:
   TessellatedComponent() = default;
   TessellatedComponent( float tessellatedEdgeSize, float tessellationFactor )
       : params( tessellatedEdgeSize, tessellationFactor )
   {
   }
   COPIABLE( TessellatedComponent );
   virtual ~TessellatedComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::TESSELLATED;

   struct ShaderParams  // GPU
   {
      ShaderParams() = default;
      ShaderParams( float tessellatedEdgeSize, float tessellationFactor )
          : tessellatedEdgeSize( tessellatedEdgeSize ), tessellationFactor( tessellationFactor )
      {
      }

      glm::vec4 frustumPlanes[6];
      glm::vec2 viewportDims;
      float tessellatedEdgeSize = 0.0f;
      float tessellationFactor  = 0.0f;
   } params;
};
}
