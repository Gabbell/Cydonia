#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <Graphics/Utility/Noise.h>

namespace CYD
{
class DisplacementComponent final : public BaseComponent
{
  public:
   DisplacementComponent() = default;
   DisplacementComponent(
       Noise::Type type,
       uint32_t width,
       uint32_t height,
       const Noise::ShaderParams& params,
       bool generateNormals = false,
       float speed          = 0.0f );
   COPIABLE( DisplacementComponent );
   virtual ~DisplacementComponent();

   static constexpr ComponentType TYPE = ComponentType::DISPLACEMENT;

   Noise::Type type           = Noise::Type::WHITE_NOISE;
   Noise::ShaderParams params = {};

   TextureHandle noiseTex;
   TextureHandle normalMap;

   uint32_t width           = 0;
   uint32_t height          = 0;
   float speed              = 0.0f;
   bool generateNormals : 1 = false;

   bool needsUpdate : 1       = true;
   bool resolutionChanged : 1 = true;
};
}
