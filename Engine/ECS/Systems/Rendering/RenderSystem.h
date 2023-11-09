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
class MeshCache;
class MaterialCache;
class SceneComponent;
struct PipelineInfo;

class RenderSystem
    : public CommonSystem<RenderableComponent, TransformComponent, MeshComponent, MaterialComponent>
{
  public:
   RenderSystem() = delete;
   RenderSystem( const MeshCache& meshes, const MaterialCache& materials )
       : m_meshes( meshes ), m_materials( materials )
   {
   }
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

   const MeshCache& m_meshes;
   const MaterialCache& m_materials;
};
}
