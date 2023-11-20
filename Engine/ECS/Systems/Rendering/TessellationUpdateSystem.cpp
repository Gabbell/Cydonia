#include <ECS/Systems/Rendering/TessellationUpdateSystem.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/SharedComponents/SceneComponent.h>
#include <ECS/EntityManager.h>

#include <Profiling.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cstdlib>

namespace CYD
{
void TessellationUpdateSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   for( const auto& entityEntry : m_entities )
   {
      RenderableComponent& renderable   = GetComponent<RenderableComponent>( entityEntry );
      TessellatedComponent& tessellated = GetComponent<TessellatedComponent>( entityEntry );

      renderable.isTessellated = true;

      const Frustum& mainViewFrustum = scene.frustums[0];

      tessellated.params.viewportDims = glm::vec2( scene.viewport.width, scene.viewport.height );
      mainViewFrustum.getPlanes( tessellated.params.frustumPlanes );

      // Creating GPU data
      const size_t bufferSize = sizeof( TessellatedComponent::ShaderParams );

      if( !renderable.tessellationBuffer )
      {
         renderable.tessellationBuffer =
             GRIS::CreateUniformBuffer( bufferSize, "Tessellation Params Buffer" );
      }

      // Transferring all the views to one buffer
      const UploadToBufferInfo info = { 0, bufferSize };
      GRIS::UploadToBuffer( renderable.tessellationBuffer, &tessellated.params, info );
   }
}
}
