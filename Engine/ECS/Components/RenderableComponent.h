#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Handles/Handle.h>

#include <Graphics/GraphicsTypes.h>

#include <glm/glm.hpp>

namespace cyd
{
struct MVP
{
   glm::mat4 model = glm::mat4( 1.0f );
   glm::mat4 view  = glm::mat4( 1.0f );
   glm::mat4 proj  = glm::mat4( 1.0f );
};

class RenderableComponent final : public BaseComponent
{
  public:
   RenderableComponent() = default;
   COPIABLE( RenderableComponent );
   virtual ~RenderableComponent();

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   bool init() override;
   void uninit() override;

   Extent extent;
   PipelineInfo pipInfo;
   PipelineLayoutInfo pipLayoutInfo;

   TextureHandle texture;
   VertexBufferHandle vertexBuffer;
   IndexBufferHandle indexBuffer;
   UniformBufferHandle uboBuffer;
};
}
