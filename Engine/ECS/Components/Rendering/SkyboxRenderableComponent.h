#pragma once

#include <ECS/Components/Rendering/RenderableComponent.h>

#include <Handles/Handle.h>

#include <ECS/Components/ComponentTypes.h>

#include <string>

namespace CYD
{
class SkyboxRenderableComponent final : public RenderableComponent
{
  public:
   SkyboxRenderableComponent() = default;
   COPIABLE( SkyboxRenderableComponent );
   virtual ~SkyboxRenderableComponent();

   bool init( const std::string& skyboxName );
   void uninit() override;

   ComponentType getType() const override { return SUBTYPE; }

   static constexpr ComponentType SUBTYPE = ComponentType::RENDERABLE_SKYBOX;

   TextureHandle cubemap;
};
}
