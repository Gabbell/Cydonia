#include <ECS/Systems/UI/ImGuiSystem.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <UI/UserInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

namespace CYD
{
static Framebuffer s_uiFB = {};

static bool s_drawECSWindow       = false;
static bool s_drawMaterialsWindow = false;
static bool s_drawPipelinesWindow = false;
static bool s_drawAboutWindow     = false;
static bool s_drawStatsOverlay    = false;

ImGuiSystem::ImGuiSystem( const EntityManager& entityManager ) : m_entityManager( entityManager )
{
   // We initialize the UI here
   // It needs to be after the WindowSystem is initialized because if we initialize it before, we override ImGui's GLFW callbacks
   UI::Initialize();
}

ImGuiSystem::~ImGuiSystem() { UI::Uninitialize(); }

void ImGuiSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "ImGuiSystem" );

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::UI );
   CYD_SCOPED_GPUTRACE( cmdList, "ImGuiSystem" );

   const SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Setting up interface
   UI::DrawMainWindow( cmdList );
   UI::DrawMainMenuBar(
       cmdList,
       s_drawECSWindow,
       s_drawMaterialsWindow,
       s_drawPipelinesWindow,
       s_drawAboutWindow,
       s_drawStatsOverlay );

   if( s_drawECSWindow )
   {
      UI::DrawECSWindow( cmdList, m_entityManager );
   }

   if( s_drawMaterialsWindow )
   {
      UI::DrawMaterialsWindow( cmdList );
   }

   if( s_drawPipelinesWindow )
   {
      UI::DrawPipelinesWindow( cmdList );
   }

   if( s_drawAboutWindow )
   {
      UI::DrawAboutWindow( cmdList );
   }

   if( s_drawStatsOverlay )
   {
      UI::DrawStatsOverlay( cmdList );
   }

   if( scene.resolutionChanged )
   {
      s_uiFB.resize( scene.mainFramebuffer.getWidth(), scene.mainFramebuffer.getHeight() );
      s_uiFB.replace( Framebuffer::COLOR, scene.mainColor, Access::PRESENT );
   }

   GRIS::BeginRendering( cmdList, s_uiFB );
   GRIS::DrawUI( cmdList );
   GRIS::EndRendering( cmdList );
}
}
