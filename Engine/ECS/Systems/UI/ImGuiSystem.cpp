#include <ECS/Systems/UI/ImGuiSystem.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <UI/UserInterface.h>

#include <Profiling.h>

namespace CYD
{
static bool s_drawECSWindow       = false;
static bool s_drawMaterialsWindow = false;
static bool s_drawPipelinesWindow = false;
static bool s_drawAboutWindow     = false;
static bool s_drawStatsOverlay    = false;

ImGuiSystem::ImGuiSystem( const EntityManager& entityManager ) : m_entityManager( entityManager )
{
   UI::Initialize();
}
ImGuiSystem::~ImGuiSystem() { UI::Uninitialize(); }

void ImGuiSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE( "ImGuiSystem" );

   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::UI );
   CYD_SCOPED_GPUTRACE( cmdList, "ImGuiSystem" );

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

   GRIS::BeginRendering( cmdList );
   GRIS::DrawUI( cmdList );
   GRIS::EndRendering( cmdList );
}
}
