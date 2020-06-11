#pragma once

#include <ECS/Components/Rendering/RenderableComponent.h>

#include <Handles/Handle.h>

#include <ECS/Components/ComponentTypes.h>

namespace CYD
{
class PhongRenderableComponent final : public RenderableComponent
{
  public:
   PhongRenderableComponent() = default;
   COPIABLE( PhongRenderableComponent );
   virtual ~PhongRenderableComponent();

   bool init();
   void uninit() override;

   ComponentType getType() const override { return SUBTYPE; }

   static constexpr ComponentType SUBTYPE = ComponentType::RENDERABLE_PHONG;

   TextureHandle texture;
};
}
