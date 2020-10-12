#include <ECS/Systems/Rendering/ForwardRenderSystem.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
ForwardRenderSystem::ForwardRenderSystem()
{
   // TODO: Will have to recall for resizing events
   // Flipping Y in viewport since we want Y to be up (like GL)
   _renderGraph.setViewport( 0, 1080, 1920, -1080 );
   _renderGraph.setScissor( 0, 0, 1920, 1080 );
}

void ForwardRenderSystem::tick( double /*deltaS*/ )
{
   const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();
   const SceneComponent& scene   = ECS::GetSharedComponent<SceneComponent>();

   _renderGraph.addView(
       RenderGraph::MAIN_VIEW_STRING, camera.pos, camera.vp.view, camera.vp.proj );

   _renderGraph.addLight( scene.dirLight.enabled, scene.dirLight.direction, scene.dirLight.color );

   // Add renderables to the render graph
   for( const auto& compPair : m_components )
   {
      const TransformComponent& transform   = *std::get<TransformComponent*>( compPair.second );
      const RenderableComponent& renderable = *std::get<RenderableComponent*>( compPair.second );
      const MeshComponent& mesh             = *std::get<MeshComponent*>( compPair.second );

      const glm::mat4 modelMatrix =
          glm::scale( glm::translate( glm::mat4( 1.0f ), transform.position ), transform.scaling ) *
          glm::toMat4( transform.rotation );

      // Add renderable entity and its shader resources to the render graph
      _renderGraph.add3DRenderable( modelMatrix, renderable.type, renderable.asset, mesh.asset );
   }

   const bool compileSuccess = _renderGraph.compile();
   const bool executeSuccess = _renderGraph.execute();

   CYDASSERT( compileSuccess && executeSuccess && "ForwardRenderSystem: Render graph failed" );

   _renderGraph.reset();
}
}
