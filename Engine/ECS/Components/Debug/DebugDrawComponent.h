#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <glm/glm.hpp>

// ================================================================================================
// Definition
// ================================================================================================
/*
This class is used to draw geometry on top of entities
*/
namespace CYD
{
class DebugDrawComponent : public BaseComponent
{
  public:
   enum class Type
   {
      SPHERE,
      NORMALS,
      TEXTURE
   };

   // =============================================================================================
   struct ShaderParams
   {
      ShaderParams() = default;
      ShaderParams( const glm::vec4& color ) : color( color ) {}

      glm::vec4 color = glm::vec4( 1.0f );
   };

   // =============================================================================================
   struct SphereParams
   {
      float radius;
   };

   union UnionParams
   {
      SphereParams sphere;
   };

   // =============================================================================================
   DebugDrawComponent() = default;
   DebugDrawComponent(
       DebugDrawComponent::Type drawType,
       glm::vec4 color = glm::vec4( 1.0f ),
       float radius    = 1.0f )
       : type( drawType ), shaderParams( color )
   {
      CYD_ASSERT( drawType == Type::SPHERE );
      params.sphere.radius = radius;
   }
   COPIABLE( DebugDrawComponent );
   virtual ~DebugDrawComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::DEBUG_DRAW;

   Type type = Type::SPHERE;

   UnionParams params;
   ShaderParams shaderParams;
};
}