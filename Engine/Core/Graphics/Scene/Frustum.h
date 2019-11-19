#pragma once

#include <glm/glm.hpp>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Frustum
{
  public:
   Frustum();
   ~Frustum() = default;

  private:
   // 0: top plane
   // 1: bottom plane
   // 2: left plane
   // 3: right plane
   // 4: near plane
   // 5: far plane
   glm::vec4 _planes[6];
};
}
