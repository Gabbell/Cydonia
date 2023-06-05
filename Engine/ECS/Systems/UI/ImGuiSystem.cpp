#include <ECS/Systems/UI/ImGuiSystem.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <UI/UserInterface.h>

namespace CYD
{
static bool s_drawECSWindow       = false;
static bool s_drawMaterialsWindow = false;
static bool s_drawPipelinesWindow = false;
static bool s_drawAboutWindow     = false;

ImGuiSystem::ImGuiSystem( const EntityManager& entityManager ) : m_entityManager( entityManager )
{
   GRIS::InitializeUI();
}
ImGuiSystem::~ImGuiSystem() { GRIS::UninitializeUI(); }

void ImGuiSystem::tick( double /*deltaS*/ )
{
   // Setting up interface
   UI::DrawMainWindow();
   UI::DrawMainMenuBar( s_drawECSWindow, s_drawMaterialsWindow, s_drawPipelinesWindow, s_drawAboutWindow );

   if( s_drawECSWindow )
   {
      UI::DrawECSWindow( m_entityManager );
   }

   if( s_drawMaterialsWindow )
   {
      UI::DrawMaterialsWindow();
   }

   if( s_drawPipelinesWindow )
   {
      UI::DrawPipelinesWindow();
   }

   if( s_drawAboutWindow )
   {
      UI::DrawAboutWindow();
   }

   // Render UI
   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, "ImGuiSystem", true );

   GRIS::StartRecordingCommandList( cmdList );

   GRIS::BeginRendering( cmdList );

   GRIS::DrawUI( cmdList );

   GRIS::EndRendering( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   RenderGraph::AddPass( cmdList );
}
}
