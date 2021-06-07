#include <ECS/Systems/Rendering/AtmosphereRenderSystem.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
static constexpr uint32_t ENV_VIEW_SET = 0;

static void setupScene( CmdListHandle cmdList )
{
   const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();
   const SceneComponent& scene   = ECS::GetSharedComponent<SceneComponent>();

   GRIS::BindUniformBuffer( cmdList, camera.viewBuffer, ENV_VIEW_SET, 0 );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );
}

void AtmosphereRenderSystem::tick( double /*deltaS*/ )
{
   if( m_entities.size() > 1 )
   {
      CYDASSERT( !"AtmosphereRenderSystem: More than one atmosphere? Huh?" );
      return;
   }

   const CmdListHandle renderList = GRIS::CreateCommandList( GRAPHICS, "Atmosphere System", true );

   for( const auto& entityEntry : m_entities )
   {
      const AtmosphereComponent& atmos = *std::get<AtmosphereComponent*>( entityEntry.arch );

      GRIS::StartRecordingCommandList( renderList );

      setupScene( renderList );

      GRIS::BeginRendering( renderList );

      GRIS::BindPipeline( renderList, "RAYLEIGH_ATMOSPHERE" );

      // GRIS::UpdateConstantBuffer(
      //     renderList, ShaderStage::FRAGMENT_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

      //GRIS::DrawFullQuad( renderList );

      GRIS::EndRendering( renderList );
   }

   GRIS::EndRecordingCommandList( renderList );

   GRIS::SubmitCommandList( renderList );
   GRIS::DestroyCommandList( renderList );
}
}
