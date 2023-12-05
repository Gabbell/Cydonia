#include <ECS/Systems/Rendering/InstanceUpdateSystem.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <Profiling.h>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cstdlib>

namespace CYD
{
void InstanceUpdateSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   std::vector<InstancedComponent::ShaderParams> ubo;
   ubo.reserve( InstancedComponent ::MAX_INSTANCES );

   for( const auto& entityEntry : m_entities )
   {
      const TransformComponent& transform = GetComponent<TransformComponent>( entityEntry );
      RenderableComponent& renderable     = GetComponent<RenderableComponent>( entityEntry );
      InstancedComponent& instanced       = GetComponent<InstancedComponent>( entityEntry );

      renderable.isInstanced = true;

      if( instanced.needsUpdate )
      {
         if( instanced.type == InstancedComponent::Type::TILED )
         {
            const uint32_t sqrtCount =
                std::round( static_cast<float>( std::sqrt( instanced.count ) ) );
            const float distance = ( sqrtCount - 1.0f ) * instanced.radius;
            const float diameter = 2.0f * instanced.radius;

            glm::vec3 position = transform.position;
            glm::vec3 scaling  = glm::vec3(1.0f);
            glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

            // Creating GPU data
            for( uint32_t instanceIdx = 0; instanceIdx < instanced.count; ++instanceIdx )
            {
               position.x = -distance + diameter * ( instanceIdx % sqrtCount );
               position.z = -distance + diameter * ( instanceIdx / sqrtCount );

               InstancedComponent::ShaderParams& shaderParams = ubo.emplace_back();

               shaderParams.modelMat = glm::toMat4( glm::conjugate( rotation ) ) *
                                       glm::scale( glm::mat4( 1.0f ), scaling ) *
                                       glm::translate( glm::mat4( 1.0f ), position );
            }
         }

         const size_t bufferSize = sizeof( InstancedComponent::ShaderParams ) * instanced.count;

         if( !renderable.instancesBuffer )
         {
            renderable.instancesBuffer =
                GRIS::CreateUniformBuffer( bufferSize, "Instances Buffer" );
         }

         // Transferring all the views to one buffer
         const UploadToBufferInfo info = { 0, bufferSize };
         GRIS::UploadToBuffer( renderable.instancesBuffer, ubo.data(), info );
         renderable.instanceCount = instanced.count;

         instanced.needsUpdate = false;
      }
   }
}
}
