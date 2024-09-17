#pragma once

#include <glm/glm.hpp>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
struct BoundingBox
{
  public:
   BoundingBox() = default;
   BoundingBox( const glm::vec3& min, const glm::vec3& max ) : min( min ), max( max ) {}
   ~BoundingBox() = default;

   glm::vec3 min = glm::vec3( -FLT_MAX );
   glm::vec3 max = glm::vec3( FLT_MAX );
};
}
