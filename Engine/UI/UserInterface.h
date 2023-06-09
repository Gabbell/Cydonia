#pragma once

#include <Graphics/Handles/ResourceHandle.h>

#include <ECS/Components/ComponentTypes.h>

namespace CYD
{
class EntityManager;
class BaseComponent;
class ProceduralDisplacementComponent;

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
void DrawProceduralDisplacementComponentMenu(
    CmdListHandle cmdList,
    const ProceduralDisplacementComponent& displacement );

// Materials
void DrawMaterialsWindow( CmdListHandle cmdList );

// Pipelines
void DrawPipelinesWindow( CmdListHandle cmdList );
}
}