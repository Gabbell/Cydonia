#include <Applications/VKShaderViewer.h>

#include <Graphics/RenderInterface.h>

#include <ECS/EntityManager.h>

#include <ECS/Systems/Rendering/RenderSystem.h>

#include <ECS/Components/Rendering/CustomRenderableComponent.h>

namespace CYD
{
// Fullscreen Quad
static std::vector<Vertex> vertices = {
    //    ~ Position ~               ~ Color ~
    {{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   //
    {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   // Top left triangle
    {{-1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  //
    {{-1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},   //
    {{1.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},    // Bottom right triangle
    {{1.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}};  //

VKShaderViewer::VKShaderViewer( uint32_t width, uint32_t height, const std::string& shaderName )
    : Application( width, height, "VKShaderViewer" ), m_fragShader( shaderName )
{
   // Core initializers
   GRIS::InitRenderBackend<VK>( *m_window );
   ECS::Initialize();
}

void VKShaderViewer::preLoop()
{
   ECS::AddSystem<RenderSystem>();

   //const EntityHandle rect = ECS::CreateEntity();
   //ECS::Assign<TransformComponent>( rect );
   //ECS::Assign<CustomRenderableComponent>( rect, vertices, m_fragShader );
}

void VKShaderViewer::tick( double deltaS )
{
   GRIS::PrepareFrame();
   ECS::Tick( deltaS );
   GRIS::PresentFrame();
}

VKShaderViewer::~VKShaderViewer()
{
   // Core uninitializers
   ECS::Uninitialize();
   GRIS::UninitRenderBackend();
}
}