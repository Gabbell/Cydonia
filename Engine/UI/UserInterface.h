#pragma once

#include <ECS/Components/ComponentTypes.h>

namespace CYD
{
class EntityManager;
class BaseComponent;
class ProceduralDisplacementComponent;

namespace UI
{
void DrawMainMenuBar(
    bool& drawECSWindow,
    bool& drawMaterialsWindow,
    bool& drawPipelinesWindow,
    bool& drawAboutWindow );

void DrawMainWindow();
void DrawAboutWindow();

// ECS
void DrawECSWindow( const EntityManager& entityManager );
void DrawComponentsMenu( ComponentType type, const BaseComponent* component );
void DrawProceduralDisplacementComponentMenu( const ProceduralDisplacementComponent& displacement );

// Materials
void DrawMaterialsWindow();

// Pipelines
void DrawPipelinesWindow();
}
}