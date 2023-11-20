#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>

#include <string_view>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class RenderableComponent final : public BaseComponent
{
  public:
   enum class Type
   {
      UNDEFINED,
      FORWARD,
      DEFERRED
   };

   struct Description
   {
      std::string_view pipelineName = "";
      Type type                     = Type::UNDEFINED;
      bool isVisible                = true;
      bool isShadowReceiving        = false;
      bool isShadowCasting          = false;
      bool useEnvironmentMap        = false;
   };

   RenderableComponent() = default;
   RenderableComponent( const Description& desc ) : desc( desc ) {}
   COPIABLE( RenderableComponent );
   virtual ~RenderableComponent();

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   Description desc;

   PipelineIndex pipelineIdx = INVALID_PIPELINE_IDX;

   BufferHandle tessellationBuffer;
   BufferHandle instancesBuffer;
   uint32_t instanceCount = 0;

   bool isInstanced   = false;
   bool isTessellated = false;
   bool isTransparent = false;
};
}
