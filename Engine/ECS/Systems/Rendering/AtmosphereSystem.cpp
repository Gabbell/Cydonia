#include <ECS/Systems/Rendering/AtmosphereSystem.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

namespace CYD
{
static constexpr uint32_t ENV_VIEW_SET = 0;

// static void setupScene( CmdListHandle cmdList )
// {
//    const CameraComponent& camera = ECS::GetSharedComponent<CameraComponent>();
//    const SceneComponent& scene   = ECS::GetSharedComponent<SceneComponent>();

//    GRIS::BindUniformBuffer( cmdList, camera.viewBuffer, ENV_VIEW_SET, 0 );

//    // Dynamic state
//    GRIS::SetViewport( cmdList, scene.viewport );
//    GRIS::SetScissor( cmdList, scene.scissor );
// }

void AtmosphereSystem::tick( double /*deltaS*/ )
{
    if( m_entities.size() > 1 )
    {
       CYDASSERT( !"AtmosphereSystem: More than one atmosphere? Huh?" );
       return;
    }

    // Finding main view
    const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

    const auto& it = std::find( scene.viewNames.begin(), scene.viewNames.end(), "MAIN" );
    if( it == scene.viewNames.end() )
    {
       // TODO WARNING
       CYDASSERT( !"Could not find main view, skipping render tick" );
       return;
    }
    const uint32_t viewIdx = static_cast<uint32_t>( std::distance( scene.viewNames.begin(), it ) );

   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, "AtmosphereSystem", true );
   
   GRIS::StartRecordingCommandList( cmdList );

   // Dynamic state
   GRIS::SetViewport( cmdList, scene.viewport );
   GRIS::SetScissor( cmdList, scene.scissor );

   for( const auto& entityEntry : m_entities )
   {
   //    const AtmosphereComponent& atmos = *std::get<AtmosphereComponent*>( entityEntry.arch );

   //    setupScene( renderList );

   //    GRIS::BeginRendering( renderList );

   //    GRIS::BindPipeline( renderList, "RAYLEIGH_ATMOSPHERE" );

   //    GRIS::UpdateConstantBuffer(
   //        renderList, ShaderStage::FRAGMENT_STAGE, 0, sizeof( modelMatrix ), &modelMatrix );

   //    GRIS::DrawFullQuad( renderList );

   //    GRIS::EndRendering( renderList );
   }

   GRIS::EndRecordingCommandList( cmdList );

   GRIS::SubmitCommandList( cmdList );
   GRIS::DestroyCommandList( cmdList );
}
}
