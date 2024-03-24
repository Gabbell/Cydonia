#include <ECS/Systems/Rendering/InstanceUpdateSystem.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Scene/Frustum.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cstdlib>

namespace CYD
{
static float HalfSpaceTest( const glm::vec3& point, const glm::vec4& plane )
{
   return ( plane.x * point.x ) + ( plane.y * point.y ) + ( plane.z * point.z ) + plane.w;
}

static bool
FrustumSphereCull( const FrustumShaderParams& frustum, const glm::vec3& center, float radius )
{
   for( uint32_t i = 0; i < Frustum::PLANE_COUNT; ++i )
   {
      const float distance = HalfSpaceTest( center, frustum.planes[i] );

      if( distance + radius < 0 )
      {
         return false;
      }
   }

   return true;
}

void InstanceUpdateSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   std::vector<InstancedComponent::ShaderParams> ubo;
   ubo.reserve( InstancedComponent ::MAX_INSTANCES );

   const SceneComponent& scene                = m_ecs->getSharedComponent<SceneComponent>();
   const FrustumShaderParams& mainViewFrustum = scene.frustums[0];

   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );
      RenderableComponent& renderable     = GetComponent<RenderableComponent>( entityEntry );
      InstancedComponent& instanced       = GetComponent<InstancedComponent>( entityEntry );

      renderable.isInstanced = true;

      if( instanced.needsUpdate )
      {
         uint32_t visibleInstances = 0;
         if( instanced.type == InstancedComponent::Type::TILED )
         {
            const uint32_t sqrtCount =
                std::round( static_cast<float>( std::sqrt( instanced.count ) ) );
            const float distance = ( sqrtCount - 1.0f ) * instanced.radius;
            const float diameter = 2.0f * instanced.radius;

            glm::vec3 position = glm::vec3( 0.0f );
            glm::vec3 scaling  = glm::vec3( 1.0f );
            glm::quat rotation = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );

            // Creating GPU data
            for( uint32_t instanceIdx = 0; instanceIdx < instanced.count; ++instanceIdx )
            {
               position.x = -distance + diameter * ( instanceIdx % sqrtCount );
               position.z = -distance + diameter * ( instanceIdx / sqrtCount );

               const glm::vec3 center = transform.position + position * transform.scaling;
               if( FrustumSphereCull( mainViewFrustum, center, 4800 ) )
               {
                  InstancedComponent::ShaderParams& shaderParams = ubo.emplace_back();

                  const glm::mat4 instanceMat = glm::toMat4( glm::conjugate( rotation ) ) *
                                                glm::scale( glm::mat4( 1.0f ), scaling ) *
                                                glm::translate( glm::mat4( 1.0f ), position );

                  shaderParams.index    = instanceIdx;
                  shaderParams.modelMat = instanceMat;
                  visibleInstances++;
               }
            }
         }

         if( !renderable.instancesBuffer )
         {
            renderable.instancesBuffer = GRIS::CreateUniformBuffer(
                sizeof( InstancedComponent::ShaderParams ) * instanced.count, "Instances Buffer" );
         }

         // Transferring all the views to one buffer
         const UploadToBufferInfo info = {
             0, sizeof( InstancedComponent::ShaderParams ) * visibleInstances };
         GRIS::UploadToBuffer( renderable.instancesBuffer, ubo.data(), info );

         renderable.maxInstanceCount = instanced.count;
         renderable.instanceCount    = visibleInstances;

         // instanced.needsUpdate = false;

         ubo.clear();
      }
   }
}
}
