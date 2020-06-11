#include <Applications/VKPBRDemo.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/MeshGeneration.h>

#include <ECS/EntityManager.h>

#include <ECS/Systems/Input/InputSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MovementSystem.h>
#include <ECS/Systems/Rendering/RenderSystem.h>
#include <ECS/Systems/Scene/CameraSystem.h>
#include <ECS/Systems/Lighting/LightSystem.h>

#include <ECS/Components/Lighting/DirectionalLightComponent.h>
#include <ECS/Components/Physics/MotionComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Rendering/PBRRenderableComponent.h>
#include <ECS/Components/Transforms/TransformComponent.h>

#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/InputComponent.h>

namespace CYD
{
VKPBRDemo::VKPBRDemo( uint32_t width, uint32_t height, const char* title )
    : Application( width, height, title )
{
   // Core initializers
   GRIS::InitRenderBackend<VK>( *m_window );
   ECS::Initialize();
}

void VKPBRDemo::preLoop()
{
   // This order is the order in which the systems will be ticked
   ECS::AddSystem<InputSystem>( *m_window );
   ECS::AddSystem<PlayerMoveSystem>();
   ECS::AddSystem<MovementSystem>();
   ECS::AddSystem<CameraSystem>();
   ECS::AddSystem<LightSystem>();
   ECS::AddSystem<RenderSystem>();

   // Creating player entity
   const EntityHandle player = ECS::CreateEntity();
   ECS::Assign<InputComponent>( player );
   ECS::Assign<TransformComponent>( player, glm::vec3( 0.0f, 0.0f, 50.0f ) );
   ECS::Assign<MotionComponent>( player );
   ECS::Assign<CameraComponent>( player );

   const EntityHandle sun = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( sun );
   ECS::Assign<DirectionalLightComponent>( sun );

   // Creating some renderable entities
   const EntityHandle helmet = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( helmet, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MeshComponent>( helmet, "Helmet" );
   ECS::Assign<PBRRenderableComponent>( helmet, "Helmet" );

   const EntityHandle torso = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( torso, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MeshComponent>( torso, "Torso" );
   ECS::Assign<PBRRenderableComponent>( torso, "Torso" );

   const EntityHandle boots = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( boots, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MeshComponent>( boots, "Boots" );
   ECS::Assign<PBRRenderableComponent>( boots, "Boots" );

   const EntityHandle backpack = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( backpack, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MeshComponent>( backpack, "Backpack" );
   ECS::Assign<PBRRenderableComponent>( backpack, "Backpack" );

   const EntityHandle gloves = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( gloves, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MeshComponent>( gloves, "Gloves" );
   ECS::Assign<PBRRenderableComponent>( gloves, "Gloves" );

   const EntityHandle pants = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( pants, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MeshComponent>( pants, "Pants" );
   ECS::Assign<PBRRenderableComponent>( pants, "Pants" );
}

void VKPBRDemo::tick( double deltaS )
{
   GRIS::PrepareFrame();
   ECS::Tick( deltaS );
   GRIS::PresentFrame();
}

VKPBRDemo::~VKPBRDemo()
{
   // Core uninitializers
   ECS::Uninitialize();
   GRIS::UninitRenderBackend();
}
}
