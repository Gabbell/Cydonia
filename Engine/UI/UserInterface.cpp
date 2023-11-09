#include <UI/UserInterface.h>
#include <UI/UserInterface.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/TessellatedComponent.h>
#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>
#include <ECS/Components/Procedural/AtmosphereComponent.h>
#include <ECS/Components/Procedural/FFTOceanComponent.h>
#include <ECS/Components/Procedural/FogComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <ThirdParty/ImGui/imgui.h>

#include <glm/gtc/type_ptr.hpp>

namespace CYD::UI
{
static ImTextureID s_oceanDispTexture = nullptr;
static ImTextureID s_noiseTexture     = nullptr;

static ImTextureID s_shadowMapTexture  = nullptr;
static ImTextureID s_albedoTexture     = nullptr;
static ImTextureID s_normalsTexture    = nullptr;
static ImTextureID s_pbrTexture        = nullptr;
static ImTextureID s_shadowMaskTexture = nullptr;

static ImTextureID s_transmittanceLUTTexture     = nullptr;
static ImTextureID s_multiScatteringLUTTexture   = nullptr;
static ImTextureID s_skyViewLUTTexture           = nullptr;
static ImTextureID s_aerialPerspectiveLUTTexture = nullptr;

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
   // ImGui::ShowDemoWindow();

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
         GRIS::ReloadShaders();
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
   const EntityManager::SharedComponents& sharedComponents = entityManager.getSharedComponents();
   for( uint32_t i = 0; i < sharedComponents.size(); ++i )
   {
      const SharedComponentType type = static_cast<SharedComponentType>( i );
      if( ImGui::TreeNodeEx( GetSharedComponentName( type ) ) )
      {
         DrawSharedComponentsMenu( cmdList, type, sharedComponents[i] );
         ImGui::TreePop();
      }
   }

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
      case ComponentType::TRANSFORM:
      {
         const TransformComponent& transform = *static_cast<const TransformComponent*>( component );
         DrawTransformComponentMenu( cmdList, transform );
         break;
      }
      case ComponentType::RENDERABLE:
      {
         const RenderableComponent& renderable =
             *static_cast<const RenderableComponent*>( component );
         DrawRenderableComponentMenu( cmdList, renderable );
         break;
      }
      case ComponentType::PROCEDURAL_DISPLACEMENT:
      {
         const ProceduralDisplacementComponent& displacement =
             *static_cast<const ProceduralDisplacementComponent*>( component );
         DrawProceduralDisplacementComponentMenu( cmdList, displacement );
         break;
      }
      case ComponentType::ATMOSPHERE:
      {
         const AtmosphereComponent& atmosphere =
             *static_cast<const AtmosphereComponent*>( component );
         DrawAtmosphereComponentMenu( cmdList, atmosphere );
         break;
      }
      case ComponentType::TESSELLATED:
      {
         const TessellatedComponent& tessellated =
             *static_cast<const TessellatedComponent*>( component );
         DrawTessellatedComponentMenu( cmdList, tessellated );
         break;
      }
      case ComponentType::FOG:
      {
         const FogComponent& fog = *static_cast<const FogComponent*>( component );
         DrawFogComponentMenu( cmdList, fog );
         break;
      }
      case ComponentType::OCEAN:
      {
         const FFTOceanComponent& ocean = *static_cast<const FFTOceanComponent*>( component );
         DrawFFTOceanComponentMenu( cmdList, ocean );
      }
      break;
      default:
         ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 255, 0, 0, 255 ) );
         ImGui::Text( "Unimplemented" );
         ImGui::PopStyleColor();
   }
}

void DrawSharedComponentsMenu(
    CmdListHandle cmdList,
    SharedComponentType type,
    const BaseSharedComponent* component )
{
   switch( type )
   {
      case SharedComponentType::SCENE:
      {
         const SceneComponent& scene = *static_cast<const SceneComponent*>( component );
         DrawSceneSharedComponentMenu( cmdList, scene );
         break;
      }
      default:
         ImGui::PushStyleColor( ImGuiCol_Text, IM_COL32( 255, 0, 0, 255 ) );
         ImGui::Text( "Unimplemented" );
         ImGui::PopStyleColor();
   }
}

void DrawTransformComponentMenu( CmdListHandle /*cmdList*/, const TransformComponent& transform )
{
   // Hello darkness, my old friend
   TransformComponent& notConst = const_cast<TransformComponent&>( transform );

   ImGui::InputFloat3( "Position (X, Y, Z)", glm::value_ptr( notConst.position ) );
   ImGui::SliderFloat3( "Scale (X, Y, Z)", glm::value_ptr( notConst.scaling ), 0.0f, 100000.0f );

   glm::vec3 eulerAngles = glm::eulerAngles( transform.rotation );  // pitch, yaw, roll
   ImGui::SliderFloat3(
       "Rotation (PITCH, YAW, ROLL)",
       glm::value_ptr( eulerAngles ),
       -3.14159265359f,
       3.14159265359f );
   notConst.rotation = glm::quat( eulerAngles );
}

void DrawProceduralDisplacementComponentMenu(
    CmdListHandle cmdList,
    const ProceduralDisplacementComponent& displacement )
{
   // Hello darkness, my old friend
   ProceduralDisplacementComponent& notConst =
       const_cast<ProceduralDisplacementComponent&>( displacement );

   ImGui::Value( "Width", displacement.width );
   ImGui::Value( "Height", displacement.height );

   bool triggerUpdate = false;

   ImGui::Value( "Seed", displacement.params.seed );
   ImGui::SameLine();

   if( ImGui::Button( "Randomize" ) )
   {
      notConst.params.seed = Noise::GenerateRandomSeed();
      triggerUpdate        = true;
   }

   if( ImGui::BeginCombo( "Noise Type", Noise::GetNoiseName( displacement.type ) ) )
   {
      for( uint32_t i = 0; i < UNDERLYING( Noise::Type::COUNT ); ++i )
      {
         if( ImGui::Selectable( Noise::GetNoiseName( static_cast<Noise::Type>( i ) ) ) )
         {
            notConst.type = static_cast<Noise::Type>( i );
            triggerUpdate = true;
         }
      }

      ImGui::EndCombo();
   }

   triggerUpdate |=
       ImGui::SliderFloat( "Amplitude", (float*)&displacement.params.amplitude, 0.0f, 2.0f );
   triggerUpdate |= ImGui::SliderFloat( "Gain", (float*)&displacement.params.gain, 0.0f, 1.0f );
   triggerUpdate |=
       ImGui::SliderFloat( "Frequency", (float*)&displacement.params.frequency, 0.0f, 10.0f );
   triggerUpdate |=
       ImGui::SliderFloat( "Lacunarity", (float*)&displacement.params.lacunarity, 1.0f, 10.0f );
   triggerUpdate |=
       ImGui::SliderFloat( "Exponent", (float*)&displacement.params.exponent, 0.0f, 5.0f );
   triggerUpdate |= ImGui::Checkbox( "Ridged", (bool*)&displacement.params.ridged );
   triggerUpdate |= ImGui::Checkbox( "Invert", (bool*)&displacement.params.invert );
   triggerUpdate |= ImGui::SliderInt( "Octaves", (int*)&displacement.params.octaves, 1, 10 );

   notConst.needsUpdate |= triggerUpdate;

   if( s_noiseTexture == nullptr )
   {
      s_noiseTexture = GRIS::AddDebugTexture( displacement.texture );
   }

   ImGui::Text( "Noise Texture" );
   float displayWidth  = ImGui::GetWindowWidth() * 0.85f;
   float displayHeight = displayWidth * static_cast<float>( displacement.height ) /
                         static_cast<float>( displacement.width );
   const ImVec2 dimensions( displayWidth, displayHeight );
   ImGui::Image( s_noiseTexture, dimensions );
}

void DrawAtmosphereComponentMenu( CmdListHandle cmdList, const AtmosphereComponent& atmosphere )
{
   AtmosphereComponent& notConst = const_cast<AtmosphereComponent&>( atmosphere );

   float displayWidth  = ImGui::GetWindowWidth() * 0.85f;
   float displayHeight = 0.0f;
   ImVec2 dimensions;

   bool triggerUpdate = false;

   ImGui::SeparatorText( "Mie" );

   triggerUpdate |=
       ImGui::SliderFloat( "Mie Phase", (float*)&atmosphere.params.miePhase, 0.0f, 1.0f );

   triggerUpdate |=
       ImGui::SliderFloat( "Mie Height", (float*)( &atmosphere.params.mieHeight ), 0.5f, 20.0f );

   triggerUpdate |= ImGui::SliderFloat(
       "Mie Scattering Scale",
       (float*)&atmosphere.params.mieScatteringCoefficient.a,
       0.00001f,
       0.1f,
       "%.5f",
       ImGuiSliderFlags_Logarithmic );

   triggerUpdate |= ImGui::ColorEdit3(
       "Mie Scattering Coefficient",
       (float*)glm::value_ptr( atmosphere.params.mieScatteringCoefficient ) );

   triggerUpdate |= ImGui::SliderFloat(
       "Mie Absorption Scale",
       (float*)&atmosphere.params.mieAbsorptionCoefficient.a,
       0.00001f,
       10.0f,
       "%.5f",
       ImGuiSliderFlags_Logarithmic );

   triggerUpdate |= ImGui::ColorEdit3(
       "Mie Absorption Coefficient",
       (float*)glm::value_ptr( atmosphere.params.mieAbsorptionCoefficient ) );

   ImGui::SeparatorText( "Rayleigh" );

   triggerUpdate |= ImGui::SliderFloat(
       "Rayleigh Height", (float*)( &atmosphere.params.rayleighHeight ), 0.5f, 20.0f );

   triggerUpdate |= ImGui::SliderFloat(
       "Rayleigh Scattering Scale",
       (float*)&atmosphere.params.rayleighScatteringCoefficient.a,
       0.00001f,
       10.0f,
       "%.5f",
       ImGuiSliderFlags_Logarithmic );

   triggerUpdate |= ImGui::ColorEdit3(
       "Rayleigh Scattering Coefficient",
       (float*)glm::value_ptr( atmosphere.params.rayleighScatteringCoefficient ) );

   ImGui::SeparatorText( "Other Absorption" );

   triggerUpdate |= ImGui::SliderFloat(
       "Absorption Scale",
       (float*)&atmosphere.params.absorptionCoefficient.a,
       0.00001f,
       10.0f,
       "%.5f",
       ImGuiSliderFlags_Logarithmic );

   triggerUpdate |= ImGui::ColorEdit3(
       "Absorption Coefficient",
       (float*)glm::value_ptr( atmosphere.params.absorptionCoefficient ) );

   ImGui::SeparatorText( "Planet Reflection" );

   triggerUpdate |= ImGui::ColorEdit3(
       "Ground Albedo", (float*)glm::value_ptr( atmosphere.params.groundAlbedo ) );

   ImGui::SeparatorText( "Height Fog" );

   ImGui::SliderFloat(
       "Height Fog Height",
       (float*)&atmosphere.params.heightFog.x,
       0.0f,
       1.0f,
       "%.5f",
       ImGuiSliderFlags_Logarithmic );
   ImGui::SliderFloat(
       "Height Fog Falloff",
       (float*)&atmosphere.params.heightFog.y,
       0.0f,
       1.0f,
       "%.5f",
       ImGuiSliderFlags_Logarithmic );

   ImGui::SliderFloat( "Height Fog Strength", (float*)&atmosphere.params.heightFog.z, 0.0f, 10.0f );

   notConst.needsUpdate = triggerUpdate;

   if( ImGui::TreeNodeEx( "Transmittance LUT" ) )
   {
      if( s_transmittanceLUTTexture == nullptr )
      {
         s_transmittanceLUTTexture = GRIS::AddDebugTexture( atmosphere.transmittanceLUT );
      }

      displayHeight = displayWidth *
                      static_cast<float>( AtmosphereComponent::TRANSMITTANCE_LUT_HEIGHT ) /
                      static_cast<float>( AtmosphereComponent::TRANSMITTANCE_LUT_WIDTH );
      dimensions = ImVec2( displayWidth, displayHeight );
      ImGui::Image( s_transmittanceLUTTexture, dimensions );

      ImGui::TreePop();
   }

   if( ImGui::TreeNodeEx( "Multiple Scattering LUT" ) )
   {
      if( s_multiScatteringLUTTexture == nullptr )
      {
         s_multiScatteringLUTTexture = GRIS::AddDebugTexture( atmosphere.multipleScatteringLUT );
      }

      displayHeight = displayWidth *
                      static_cast<float>( AtmosphereComponent::MULTIPLE_SCATTERING_LUT_DIM ) /
                      static_cast<float>( AtmosphereComponent::MULTIPLE_SCATTERING_LUT_DIM );
      dimensions = ImVec2( displayWidth, displayHeight );
      ImGui::Image( s_multiScatteringLUTTexture, dimensions );

      ImGui::TreePop();
   }

   if( ImGui::TreeNodeEx( "Sky View LUT" ) )
   {
      if( s_skyViewLUTTexture == nullptr )
      {
         s_skyViewLUTTexture = GRIS::AddDebugTexture( atmosphere.skyViewLUT );
      }

      displayHeight = displayWidth * static_cast<float>( AtmosphereComponent::SKYVIEW_LUT_HEIGHT ) /
                      static_cast<float>( AtmosphereComponent::SKYVIEW_LUT_WIDTH );
      dimensions = ImVec2( displayWidth, displayHeight );
      ImGui::Image( s_skyViewLUTTexture, dimensions );

      ImGui::TreePop();
   }
}

void DrawFFTOceanComponentMenu( CmdListHandle cmdList, const FFTOceanComponent& ocean )
{
   bool triggerUpdate = false;

   triggerUpdate |=
       ImGui::SliderFloat( "Amplitude", (float*)&ocean.params.amplitude, 0.0f, 200.0f );
   triggerUpdate |=
       ImGui::SliderFloat( "Wind Speed", (float*)&ocean.params.windSpeed, 0.0f, 100.0f );
   triggerUpdate |=
       ImGui::SliderInt( "Horizontal Dimension", (int*)&ocean.params.horizontalDimension, 0, 5000 );
   triggerUpdate |= ImGui::SliderFloat( "Wind Dir X", (float*)&ocean.params.windDirX, 0.0f, 1.0f );
   triggerUpdate |= ImGui::SliderFloat( "Wind Dir Z", (float*)&ocean.params.windDirZ, 0.0f, 1.0f );
   triggerUpdate |= ImGui::SliderFloat(
       "Horizontal Scaling", (float*)&ocean.params.horizontalScale, 0.0f, 50.0f );
   triggerUpdate |=
       ImGui::SliderFloat( "Vertical Scaling", (float*)&ocean.params.verticalScale, 0.0f, 50.0f );

   float displayWidth  = ImGui::GetWindowWidth() * 0.85f;
   float displayHeight = displayWidth * static_cast<float>( ocean.params.resolution ) /
                         static_cast<float>( ocean.params.resolution );
   const ImVec2 dimensions( displayWidth, displayHeight );

   ImGui::SeparatorText( "Displacement Map" );
   if( s_oceanDispTexture == nullptr )
   {
      s_oceanDispTexture = GRIS::AddDebugTexture( ocean.displacementMap );
   }

   ImGui::Image( s_oceanDispTexture, dimensions );

   FFTOceanComponent& notConst = const_cast<FFTOceanComponent&>( ocean );
   notConst.needsUpdate        = triggerUpdate;
}

void DrawFogComponentMenu( CmdListHandle cmdList, const FogComponent& fog )
{
   ImGui::SliderFloat( "A", (float*)&fog.params.a, 0.0f, 5.0f );
   ImGui::SliderFloat( "B", (float*)&fog.params.b, 0.0f, 5.0f );
   ImGui::SliderFloat( "Start Fog", (float*)&fog.params.startFog, 0.0f, 1000.0f );
   ImGui::SliderFloat( "End Fog", (float*)&fog.params.endFog, 0.0f, 10000.0f );
}

void DrawRenderableComponentMenu( CmdListHandle cmdList, const RenderableComponent& renderable )
{
   ImGui::Checkbox( "Is Visible", (bool*)&renderable.desc.isVisible );
   ImGui::Checkbox( "Casts Shadows", (bool*)&renderable.desc.isShadowCasting );
   ImGui::Checkbox( "Receives Shadows", (bool*)&renderable.desc.isShadowReceiving );
}

void DrawTessellatedComponentMenu( CmdListHandle cmdList, const TessellatedComponent& tessellated )
{
   ImGui::SliderFloat(
       "Tessellated Edge Size", (float*)&tessellated.params.tessellatedEdgeSize, 0.0f, 1.0f );
   ImGui::SliderFloat(
       "Tessellation Factor", (float*)&tessellated.params.tessellationFactor, 0.0f, 1.0f );
}

void DrawSceneSharedComponentMenu( CmdListHandle cmdList, const SceneComponent& scene )
{
   const GBuffer::RenderTargets& rts = scene.gbuffer.getRenderTargets();

   float displayWidth  = ImGui::GetWindowWidth() * 0.85f;
   float displayHeight = 0.0f;
   ImVec2 dimensions;

   if( ImGui::TreeNodeEx( "Shadow Map Texture", ImGuiTreeNodeFlags_SpanAvailWidth ) )
   {
      if( s_shadowMapTexture == nullptr )
      {
         s_shadowMapTexture = GRIS::AddDebugTexture( scene.shadowMap );
      }

      displayHeight = displayWidth * static_cast<float>( 2048.0f ) / static_cast<float>( 2048.0f );
      dimensions    = ImVec2( displayWidth, displayHeight );
      ImGui::Image( s_shadowMapTexture, dimensions );
      ImGui::TreePop();
   }

   displayHeight = displayWidth * static_cast<float>( scene.extent.height ) /
                   static_cast<float>( scene.extent.width );
   dimensions = ImVec2( displayWidth, displayHeight );

   if( ImGui::TreeNodeEx( "GBuffer Albedo", ImGuiTreeNodeFlags_SpanAvailWidth ) )
   {
      if( s_albedoTexture == nullptr )
      {
         s_albedoTexture = GRIS::AddDebugTexture( rts[GBuffer::ALBEDO].texture );
      }

      ImGui::Image( s_albedoTexture, dimensions );
      ImGui::TreePop();
   }

   if( ImGui::TreeNodeEx( "GBuffer Normals", ImGuiTreeNodeFlags_SpanAvailWidth ) )
   {
      if( s_normalsTexture == nullptr )
      {
         s_normalsTexture = GRIS::AddDebugTexture( rts[GBuffer::NORMAL].texture );
      }

      ImGui::Image( s_normalsTexture, dimensions );
      ImGui::TreePop();
   }

   if( ImGui::TreeNodeEx( "GBuffer PBR", ImGuiTreeNodeFlags_SpanAvailWidth ) )
   {
      if( s_pbrTexture == nullptr )
      {
         s_pbrTexture = GRIS::AddDebugTexture( rts[GBuffer::PBR].texture );
      }

      ImGui::Image( s_pbrTexture, dimensions );
      ImGui::TreePop();
   }

   if( ImGui::TreeNodeEx( "GBuffer Shadow Mask", ImGuiTreeNodeFlags_SpanAvailWidth ) )
   {
      if( s_shadowMaskTexture == nullptr )
      {
         s_shadowMaskTexture = GRIS::AddDebugTexture( rts[GBuffer::SHADOW].texture );
      }

      ImGui::Image( s_shadowMaskTexture, dimensions );
      ImGui::TreePop();
   }
}

void DrawMaterialsWindow( CmdListHandle /*cmdList*/ )
{
   ImGui::Begin( "Materials" );
   ImGui::Text( "LOL" );
   ImGui::End();
}

void DrawPipelinesWindow() {}
}