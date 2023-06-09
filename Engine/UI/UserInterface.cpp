#include <UI/UserInterface.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>

#include <ThirdParty/ImGui/imgui.h>

namespace CYD::UI
{
static ImTextureID s_noiseTexture = nullptr;

void Initialize() { GRIS::InitializeUIBackend(); }

void Uninitialize() { GRIS::UninitializeUIBackend(); }

void DrawMainMenuBar(
    CmdListHandle cmdList,
    bool& drawECSWindow,
    bool& drawMaterialsWindow,
    bool& drawPipelinesWindow,
    bool& drawAboutWindow,
    bool& drawStatsOverlay )
{
   ImGui::ShowDemoWindow();

   ImGui::BeginMainMenuBar();
   ImGui::MenuItem( "CYDONIA", nullptr, false, false );

   // Main Menu
   if( ImGui::BeginMenu( "Menu" ) )
   {
      if( ImGui::MenuItem( "Settings" ) )
      {
         //
      }
      ImGui::EndMenu();
   }
   //

   // ECS Menu
   ImGui::MenuItem( "ECS", nullptr, &drawECSWindow );
   //

   // Materials Menu
   ImGui::MenuItem( "Materials", nullptr, &drawMaterialsWindow );
   //

   // Pipelines Menu
   ImGui::MenuItem( "Pipelines", nullptr, &drawPipelinesWindow );
   //

   // Debug Menu
   if( ImGui::BeginMenu( "Debug" ) )
   {
      ImGui::MenuItem( "Statistics", "Ctrl-T", &drawStatsOverlay );

      if( ImGui::MenuItem( "Reload Shaders", "Ctrl-R" ) )
      {
         // GRIS::ReloadShaders();
      }

      ImGui::EndMenu();
   }

   // About Menu
   ImGui::MenuItem( "About", nullptr, &drawAboutWindow );
   //

   ImGui::EndMainMenuBar();
}

void DrawMainWindow( CmdListHandle /*cmdList*/ ) {}

void DrawAboutWindow( CmdListHandle /*cmdList*/ )
{
   const ImGuiIO& io = ImGui::GetIO();

   ImGui::SetNextWindowPos(
       ImVec2( io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f ),
       ImGuiCond_Always,
       ImVec2( 0.5f, 0.5f ) );

   ImGui::Begin(
       "About",
       nullptr,
       ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
           ImGuiWindowFlags_AlwaysAutoResize );

   ImGui::Text( "CYDONIA" );
   ImGui::Separator();
   ImGui::Text( "Version: 1.0.0" );

   ImGui::End();
}

void DrawStatsOverlay( CmdListHandle /*cmdList*/ )
{
   ImGui::SetNextWindowBgAlpha( 0.25f );

   ImGui::Begin(
       "Stats", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize );

   ImGui::Text( "Statistics" );

   ImGui::Separator();

   const ImGuiIO& io = ImGui::GetIO();
   ImGui::Text( "Frametime: %.3f ms (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate );
   ImGui::Text( "Command Buffers: [0: %d, 1: %d]", 0, 0 );

   ImGui::End();
}

void DrawECSWindow( CmdListHandle cmdList, const EntityManager& entityManager )
{
   ImGui::Begin( "ECS (Entity Manager)" );

   // Shared Components
   ImGui::SeparatorText( "Shared Components" );

   // Entity Components
   ImGui::SeparatorText( "Entities" );
   const EntityManager::Entities& entitiesMap = entityManager.getEntities();
   for( const auto& entityPair : entitiesMap )
   {
      const std::string entryString =
          entityPair.second.getName() + " (ID " + std::to_string( entityPair.first ) + ")";
      if( ImGui::TreeNodeEx( entryString.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth ) )
      {
         ImGui::SeparatorText( "Components" );

         const Entity::ComponentsMap& componentsMap = entityPair.second.getComponents();
         for( const auto& componentsPair : componentsMap )
         {
            if( ImGui::TreeNodeEx(
                    GetComponentName( componentsPair.first ), ImGuiTreeNodeFlags_SpanAvailWidth ) )
            {
               DrawComponentsMenu( cmdList, componentsPair.first, componentsPair.second );
               ImGui::TreePop();
            }
         }

         ImGui::TreePop();
      }
   }

   ImGui::End();
}

void DrawComponentsMenu( CmdListHandle cmdList, ComponentType type, const BaseComponent* component )
{
   switch( type )
   {
      case ComponentType::PROCEDURAL_DISPLACEMENT:
      {
         const ProceduralDisplacementComponent& displacement =
             *static_cast<const ProceduralDisplacementComponent*>( component );
         DrawProceduralDisplacementComponentMenu( cmdList, displacement );
         break;
      }
      default:
         ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 255, 0, 0, 255 ) );
         ImGui::Text( "Unimplemented" );
         ImGui::PopStyleColor();
   }
}

void DrawProceduralDisplacementComponentMenu(
    CmdListHandle cmdList,
    const ProceduralDisplacementComponent& displacement )
{
   // Hello darkness, my old friend
   ProceduralDisplacementComponent& notConst =
       const_cast<ProceduralDisplacementComponent&>( displacement );

   ImGui::Value( "Width", displacement.params.width );
   ImGui::Value( "Height", displacement.params.height );

   bool triggerUpdate = false;

   ImGui::Value( "Seed", displacement.params.seed );
   ImGui::SameLine();

   if( ImGui::Button( "Randomize" ) )
   {
      notConst.params.seed = Noise::GenerateRandomSeed();
      triggerUpdate        = true;
   }
   triggerUpdate |= ImGui::SliderFloat( "Amplitude", &notConst.params.amplitude, 0.0f, 1.0f );
   triggerUpdate |= ImGui::SliderFloat( "Gain", &notConst.params.gain, 0.0f, 1.0f );
   triggerUpdate |= ImGui::SliderFloat( "Frequency", &notConst.params.frequency, 0.0f, 10.0f );
   triggerUpdate |= ImGui::SliderFloat( "Lacunarity", &notConst.params.lacunarity, 1.0f, 10.0f );
   triggerUpdate |= ImGui::SliderFloat( "Exponent", &notConst.params.exponent, 0.0f, 5.0f );
   triggerUpdate |= ImGui::Checkbox( "Ridged", (bool*)&notConst.params.ridged );
   triggerUpdate |= ImGui::Checkbox( "Invert", (bool*)&notConst.params.invert );
   triggerUpdate |= ImGui::SliderInt( "Octaves", (int*)&notConst.params.octaves, 1, 10 );

   notConst.needsUpdate |= triggerUpdate;

   if( s_noiseTexture == nullptr )
   {
      s_noiseTexture = GRIS::AddDebugTexture( displacement.texture );
   }
   
   GRIS::UpdateDebugTexture( cmdList, displacement.texture );

   ImGui::Text( "Noise Texture" );
   float displayWidth  = ImGui::GetWindowWidth() * 0.85f;
   float displayHeight = displayWidth * static_cast<float>( displacement.params.height ) /
                         static_cast<float>( displacement.params.width );
   const ImVec2 dimensions( displayWidth, displayHeight );
   ImGui::Image( s_noiseTexture, dimensions );
}

void DrawMaterialsWindow(CmdListHandle /*cmdList*/)
{
   ImGui::Begin( "Materials" );
   ImGui::Text( "LOL" );
   ImGui::End();
}

void DrawPipelinesWindow(CmdListHandle /*cmdList*/)
{
   //
}
}