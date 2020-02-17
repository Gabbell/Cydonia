#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Handles/Handle.h>

namespace cyd
{
class RenderableComponent final : public BaseComponent
{
  public:
   RenderableComponent() = default;
   COPIABLE( RenderableComponent );
   virtual ~RenderableComponent();

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   bool init() override;
   void uninit() override;

   UniformBufferHandle matBuffer;  // Material properties
   TextureHandle matTexture;

   VertexBufferHandle vertexBuffer;  // Mesh data
   IndexBufferHandle indexBuffer;
};
}
