#pragma once

#include <ECS/Systems/CommonSystem.h>

#include <Common/Include.h>

#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class MaterialCache;
class SceneComponent;
struct PipelineInfo;

class RenderSystem
    : public CommonSystem<RenderableComponent, TransformComponent, MeshComponent, MaterialComponent>
{
  public:
   RenderSystem() = delete;
   RenderSystem( const MaterialCache& materials ) : m_materials( materials ) {}
   NON_COPIABLE( RenderSystem );
   virtual ~RenderSystem() = default;

  protected:
   uint32_t getViewIndex( const SceneComponent& scene, std::string_view name ) const;

   void bindView(
       CmdListHandle cmdList,
       const SceneComponent& scene,
       const PipelineInfo* pipInfo,
       uint32_t viewIndex ) const;

   void bindInverseView(
       CmdListHandle cmdList,
       const SceneComponent& scene,
       const PipelineInfo* pipInfo,
       uint32_t viewIndex ) const;

   const MaterialCache& m_materials;
};
}
