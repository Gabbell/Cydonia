#pragma once

#include <ECS/Components/BaseComponent.h>
#include <ECS/Components/ComponentTypes.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>

namespace CYD
{
class NoiseComponent final : public BaseComponent
{
  public:
   NoiseComponent() = default;
   NoiseComponent( bool isConstant ) : constant( isConstant ) {}
   COPIABLE( NoiseComponent );
   virtual ~NoiseComponent();

   static constexpr ComponentType TYPE = ComponentType::NOISE;

   struct ShaderParams
   {
      float width;
      float height;
      float seed;
      float frequency;
      float scale;
      float exponent;
      uint32_t normalize;
      uint32_t absolute;
      uint32_t octaves;
   } params;
   TextureHandle texture;

   // If constant, this noise will not be regenerated every frame. Only in the case of resizing
   bool constant : 1    = true;
   bool needsUpdate : 1 = true;
};
}