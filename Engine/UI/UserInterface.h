#pragma once

#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/Components/ComponentTypes.h>
#include <ECS/SharedComponents/SharedComponentType.h>

namespace CYD
{
class EntityManager;
class BaseComponent;
class BaseSharedComponent;
class TransformComponent;
class RenderableComponent;
class TessellatedComponent;
class ProceduralDisplacementComponent;
class AtmosphereComponent;
class FFTOceanComponent;
class FogComponent;
class SceneComponent;

namespace UI
{
void Initialize();
void Uninitialize();

// Main Menus
void DrawMainMenuBar(
    CmdListHandle cmdList,
    bool& drawECSWindow,
    bool& drawMaterialsWindow,
    bool& drawPipelinesWindow,
    bool& drawAboutWindow,
    bool& drawStatsOverlay );

void DrawMainWindow( CmdListHandle cmdList );
void DrawAboutWindow( CmdListHandle cmdList );
void DrawStatsOverlay( CmdListHandle cmdList );

// ECS
void DrawECSWindow( CmdListHandle cmdList, const EntityManager& entityManager );
void DrawComponentsMenu(
    CmdListHandle cmdList,
    ComponentType type,
    const BaseComponent* component );
void DrawSharedComponentsMenu(
    CmdListHandle cmdList,
    SharedComponentType type,
    const BaseSharedComponent* component );
void DrawTransformComponentMenu( CmdListHandle cmdList, const TransformComponent& transform );
void DrawRenderableComponentMenu( CmdListHandle cmdList, const RenderableComponent& renderable );
void DrawTessellatedComponentMenu( CmdListHandle cmdList, const TessellatedComponent& tessellated );
void DrawProceduralDisplacementComponentMenu(
    CmdListHandle cmdList,
    const ProceduralDisplacementComponent& displacement );
void DrawAtmosphereComponentMenu( CmdListHandle cmdList, const AtmosphereComponent& atmosphere );
void DrawFFTOceanComponentMenu( CmdListHandle cmdList, const FFTOceanComponent& ocean );
void DrawFogComponentMenu( CmdListHandle cmdList, const FogComponent& fog );
void DrawSceneSharedComponentMenu( CmdListHandle cmdList, const SceneComponent& scene );

// Materials
void DrawMaterialsWindow( CmdListHandle cmdList );

// Pipelines
void DrawPipelinesWindow( CmdListHandle cmdList );
}
}