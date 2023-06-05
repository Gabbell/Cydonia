#include <UI/UserInterface.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>

#include <ThirdParty/ImGui/imgui.h>

namespace CYD::UI
{
static TextureHandle s_cydoniaHandle = {};
static ImTextureID s_cydoniaLogo     = nullptr;
static ImTextureID s_noiseTexture    = nullptr;

static const char* GetComponentName( ComponentType type )
{
   static constexpr char COMPONENT_NAMES[static_cast<size_t>( ComponentType::COUNT )][32] = {
       "Transform",
       "Camera",
       "Light",
       "Material",
       "Mesh",
       "Renderable",
       "Fullscreen",
       "Procedural Displacement",
       "Ocean",
       "Atmosphere",
       "Motion",
       "Entity Follow",
       "Debug Draw",
       "Debug Sphere" };

   return COMPONENT_NAMES[static_cast<size_t>( type )];
}

void DrawMainMenuBar(
    bool& drawECSWindow,
    bool& drawMaterialsWindow,
    bool& drawPipelinesWindow,
    bool& drawAboutWindow )
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

void DrawMainWindow()
{
   // const ImGuiIO& io = ImGui::GetIO();
   // ImVec2 windowSize = io.DisplaySize;

   // ImGui::SetNextWindowSize( windowSize );
   // ImGui::SetNextWindowPos( { 0.0f, 0.0f } );
   // ImGui::Begin( "Main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground
   // );

   // windowSize.x *= 0.1f;
   // windowSize.y *= 0.33f;
   // ImGui::BeginChild( "Cydonia", windowSize, true );
   // ImGui::Text( "Hello" );
   // ImGui::EndChild();

   // ImGui::End();
}

void DrawAboutWindow()
{
   ImGui::Begin( "About" );

   ImGui::End();
}

void DrawECSWindow( const EntityManager& entityManager )
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
               DrawComponentsMenu( componentsPair.first, componentsPair.second );
               ImGui::TreePop();
            }
         }

         ImGui::TreePop();
      }
   }

   ImGui::End();
}

void DrawComponentsMenu( ComponentType type, const BaseComponent* component )
{
   switch( type )
   {
      case ComponentType::PROCEDURAL_DISPLACEMENT:
      {
         const ProceduralDisplacementComponent& displacement =
             *static_cast<const ProceduralDisplacementComponent*>( component );
         DrawProceduralDisplacementComponentMenu( displacement );
         break;
      }
      default:
         ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 255, 0, 0, 255 ) );
         ImGui::Text( "Unimplemented" );
         ImGui::PopStyleColor();
   }
}

void DrawProceduralDisplacementComponentMenu( const ProceduralDisplacementComponent& displacement )
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
   triggerUpdate |= ImGui::Checkbox( "Absolute", (bool*)&notConst.params.absolute );
   triggerUpdate |= ImGui::Checkbox( "Invert", (bool*)&notConst.params.invert );
   triggerUpdate |= ImGui::SliderInt( "Octaves", (int*)&notConst.params.octaves, 1, 10 );

   notConst.needsUpdate |= triggerUpdate;

   if( s_noiseTexture )
   {
      GRIS::RemoveDebugTexture( s_noiseTexture );
      s_noiseTexture = nullptr;
   }

   if( s_noiseTexture == nullptr )
   {
      s_noiseTexture = GRIS::AddDebugTexture( displacement.texture );
   }

   ImGui::Text( "Noise Texture" );
   float displayWidth  = ImGui::GetWindowWidth() * 0.85f;
   float displayHeight = displayWidth * static_cast<float>( displacement.params.height ) /
                         static_cast<float>( displacement.params.width );
   const ImVec2 dimensions( displayWidth, displayHeight );
   ImGui::Image( s_noiseTexture, dimensions );
}

void DrawMaterialsWindow()
{
   ImGui::Begin( "Materials" );
   ImGui::Text( "LOL" );
   ImGui::End();
}

void DrawPipelinesWindow()
{
   //
}
}