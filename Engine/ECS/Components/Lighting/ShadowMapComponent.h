#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/Utility/ShadowMapping.h>

#include <glm/glm.hpp>

namespace CYD
{
class ShadowMapComponent final : public BaseComponent
{
  public:
   enum class Type
   {
      BASIC,
      VARIANCE
   };

   ShadowMapComponent() = default;
   ShadowMapComponent( uint32_t resolution )
       : resolution( std::max( resolution, ShadowMapping::MAX_RESOLUTION ) )
   {
   }
   COPIABLE( ShadowMapComponent );
   virtual ~ShadowMapComponent();

   static constexpr ComponentType TYPE = ComponentType::SHADOW_MAP;

   using ShadowMapIndex                                    = uint32_t;
   static constexpr ShadowMapIndex INVALID_SHADOWMAP_INDEX = 0xFFFFFFFF;
   uint32_t index                                          = INVALID_SHADOWMAP_INDEX;

   Type type = Type::BASIC;

   TextureHandle depth;
   TextureHandle filterable;

   uint32_t resolution  = ShadowMapping::MAX_RESOLUTION;
   uint32_t numCascades = ShadowMapping::MAX_CASCADES;
   float splitLambda    = 0.5f;
   bool isFilterable    = false;

   bool enabled = true;
};
}