#include <ECS/Systems/UI/ImGuiSystem.h>

#include <Graphics/RenderGraph.h>

#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Handles/ResourceHandle.h>

#include <ThirdParty/ImGui/imgui.h>

namespace CYD
{
void ImGuiSystem::tick( double /*deltaS*/ )
{
   ImGui::ShowDemoWindow();

   const CmdListHandle cmdList = GRIS::CreateCommandList( GRAPHICS, "ImGuiSystem", true );

   GRIS::StartRecordingCommandList( cmdList );

   GRIS::BeginRendering( cmdList );

   GRIS::DrawUI( cmdList );

   GRIS::EndRendering( cmdList );

   GRIS::EndRecordingCommandList( cmdList );

   RenderGraph::AddPass( cmdList );
}
}
