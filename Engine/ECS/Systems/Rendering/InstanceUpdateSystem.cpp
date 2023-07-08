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
   CYD_TRACE( "InstanceUpdateSystem" );

   std::vector<InstancedComponent::ShaderParams> ubo;
   ubo.reserve( InstancedComponent ::MAX_INSTANCES );

   for( const auto& entityEntry : m_entities )
   {
      RenderableComponent& renderable = *std::get<RenderableComponent*>( entityEntry.arch );
      InstancedComponent& instanced   = *std::get<InstancedComponent*>( entityEntry.arch );

      renderable.isInstanced = true;

      if( instanced.needsUpdate )
      {
         // Creating GPU data
         for( uint32_t instanceIdx = 0; instanceIdx < instanced.count; ++instanceIdx )
         {
            glm::vec3 position =
                glm::vec3( std::rand() % 100 - 50, std::rand() % 100 - 50, std::rand() % 100 - 50 );
            glm::vec3 scaling  = glm::vec3( 1.0f );
            glm::quat rotation = glm::quat( 1.0f, 0.0f, 0.0f, 0.0f );

            InstancedComponent::ShaderParams& shaderParams = ubo.emplace_back();

            shaderParams.modelMat = glm::toMat4( glm::conjugate( rotation ) ) *
                                    glm::scale( glm::mat4( 1.0f ), glm::vec3( 1.0f ) / scaling ) *
                                    glm::translate( glm::mat4( 1.0f ), position );
         }

         const size_t bufferSize = sizeof( InstancedComponent::ShaderParams ) * instanced.count;

         if( !renderable.instancesBuffer )
         {
            renderable.instancesBuffer =
                GRIS::CreateUniformBuffer( bufferSize, "Instances Buffer" );
         }

         // Transferring all the views to one buffer
         GRIS::CopyToBuffer( renderable.instancesBuffer, ubo.data(), 0, bufferSize );
         renderable.instanceCount = instanced.count;

         instanced.needsUpdate = false;
      }
   }
}
}
