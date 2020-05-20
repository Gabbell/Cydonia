#pragma once

#include <ECS/Components/Rendering/RenderableComponent.h>

#include <Handles/Handle.h>

namespace CYD
{
class PhongRenderableComponent final : public RenderableComponent
{
  public:
   PhongRenderableComponent();
   COPIABLE( PhongRenderableComponent );
   virtual ~PhongRenderableComponent();

   bool init();
   void uninit() override;

   TextureHandle texture;
};
}
