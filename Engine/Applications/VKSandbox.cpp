#include <Applications/VKSandbox.h>

#include <Graphics/RenderInterface.h>

#include <ECS/ECS.h>

#include <ECS/Systems/InputSystem.h>
#include <ECS/Systems/PlayerMoveSystem.h>
#include <ECS/Systems/RenderSystem.h>

#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/MotionComponent.h>
#include <ECS/Components/RenderableComponent.h>
#include <ECS/SharedComponents/InputComponent.h>

namespace cyd
{
VKSandbox::VKSandbox() = default;

bool VKSandbox::init( uint32_t width, uint32_t height, const std::string& title )
{
   Application::init( width, height, title );

   GRIS::InitRenderBackend<VK>( *m_window );
   ECS::Initialize();

   return true;
}

void VKSandbox::preLoop()
{
   // This order is the order in which the systems will be ticked
   ECS::AddSystem<InputSystem>( *m_window );
   ECS::AddSystem<PlayerMoveSystem>();
   ECS::AddSystem<RenderSystem>();

   // Creating player entity
   const EntityHandle player = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( player );
   ECS::Assign<InputComponent>( player );
   ECS::Assign<MotionComponent>( player );

   // Creating some renderable entities
   for( uint32_t i = 0; i < 5; ++i )
   {
     const EntityHandle triangle = ECS::CreateEntity();
     ECS::Assign<TransformComponent>(
         triangle,
         glm::vec3( i / 100.0f, 0.0f, 0.0f ),
         glm::vec3( 1.0f ),
         glm::identity<glm::quat>() );
     ECS::Assign<RenderableComponent>( triangle );
   }
}

void VKSandbox::tick( double deltaS ) { ECS::Tick( deltaS ); }

void VKSandbox::postLoop() {}

VKSandbox::~VKSandbox() { ECS::Uninitialize(); }
}
