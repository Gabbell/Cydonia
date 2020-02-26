#include <Applications/VKSandbox.h>

#include <Graphics/RenderInterface.h>

#include <ECS/ECS.h>

#include <ECS/Systems/InputSystem.h>
#include <ECS/Systems/PlayerMoveSystem.h>
#include <ECS/Systems/MovementSystem.h>
#include <ECS/Systems/CameraSystem.h>
#include <ECS/Systems/RenderSystem.h>

#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/MotionComponent.h>
#include <ECS/Components/RenderableComponent.h>
#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/InputComponent.h>

namespace cyd
{
// Plane
static std::vector<Vertex> vertices = {
    //    ~ Position ~               ~ Color ~                ~ UV ~            ~ Normal ~
    {{999.0f, 0.0f, 999.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-999.0f, 0.0f, -999.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-999.0f, 0.0f, 999.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{999.0f, 0.0f, 999.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{999.0f, 0.0f, -999.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    {{-999.0f, 0.0f, -999.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}};

VKSandbox::VKSandbox() = default;

bool VKSandbox::init( uint32_t width, uint32_t height, const std::string& title )
{
   Application::init( width, height, title );

   bool success = true;

   success &= GRIS::InitRenderBackend<VK>( *m_window );
   success &= ECS::Initialize();

   return success;
}

void VKSandbox::preLoop()
{
   // This order is the order in which the systems will be ticked
   ECS::AddSystem<InputSystem>( *m_window );
   ECS::AddSystem<PlayerMoveSystem>();
   ECS::AddSystem<MovementSystem>();
   ECS::AddSystem<CameraSystem>();
   ECS::AddSystem<RenderSystem>();

   // Creating player entity
   const EntityHandle player = ECS::CreateEntity();
   ECS::Assign<InputComponent>( player );
   ECS::Assign<TransformComponent>( player );
   ECS::Assign<MotionComponent>( player );
   ECS::Assign<CameraComponent>( player );

   // Creating some renderable entities
   const EntityHandle sphere = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( sphere );
   ECS::Assign<RenderableComponent>( sphere, "Assets/Meshes/sphere.obj" );

   const EntityHandle plane = ECS::CreateEntity();
   ECS::Assign<TransformComponent>(
       plane, glm::vec3( 0.0f, -20.0f, 0.0f ), glm::vec3(1.0f), glm::quat() );
   ECS::Assign<RenderableComponent>( plane, vertices );
}

void VKSandbox::tick( double deltaS ) { ECS::Tick( deltaS ); }

void VKSandbox::postLoop() {}

VKSandbox::~VKSandbox()
{
   ECS::Uninitialize();
   GRIS::UninitRenderBackend();
}
}  // namespace cyd
