#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <Graphics/Utility/Noise.h>

namespace CYD
{
class ProceduralDisplacementComponent final : public BaseComponent
{
  public:
   ProceduralDisplacementComponent() = default;
   ProceduralDisplacementComponent(
       Noise::Type type,
       uint32_t width,
       uint32_t height,
       const Noise::ShaderParams& params );
   ProceduralDisplacementComponent(
       Noise::Type type,
       uint32_t width,
       uint32_t height,
       float speed );
   COPIABLE( ProceduralDisplacementComponent );
   virtual ~ProceduralDisplacementComponent();

   static constexpr ComponentType TYPE = ComponentType::PROCEDURAL_DISPLACEMENT;

   float scale = 1.0f;

   Noise::Type type           = Noise::Type::WHITE_NOISE;
   Noise::ShaderParams params = {};

   float speed     = 0.0f;
   uint32_t width  = 0;
   uint32_t height = 0;

   TextureHandle texture;

   bool needsUpdate : 1       = true;
   bool resolutionChanged : 1 = true;
};
}
