#include <Applications/VKSandbox.h>

#include <Graphics/RenderInterface.h>

#include <ECS/Systems/Lighting/LightSystem.h>
#include <ECS/Systems/Input/InputSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MotionSystem.h>
#include <ECS/Systems/Rendering/ForwardRenderSystem.h>
#include <ECS/Systems/Scene/CameraSystem.h>

#include <ECS/Components/Lighting/LightComponent.h>
#include <ECS/Components/Physics/MotionComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Transforms/TransformComponent.h>

#include <ECS/SharedComponents/CameraComponent.h>
#include <ECS/SharedComponents/InputComponent.h>

#include <ECS/EntityManager.h>

namespace CYD
{
VKSandbox::VKSandbox( uint32_t width, uint32_t height, const char* title )
    : Application( width, height, title )
{
   // Core initializers
   GRIS::InitRenderBackend<VK>( *m_window );
   ECS::Initialize();
}

void VKSandbox::preLoop()
{
   ECS::AddSystem<InputSystem>( *m_window );
   ECS::AddSystem<CameraSystem>();
   ECS::AddSystem<PlayerMoveSystem>();
   ECS::AddSystem<MotionSystem>();
   ECS::AddSystem<LightSystem>();
   ECS::AddSystem<ForwardRenderSystem>();

   // Creating player entity
   const EntityHandle player = ECS::CreateEntity();
   ECS::Assign<InputComponent>( player );
   ECS::Assign<TransformComponent>( player, glm::vec3( 0.0f, 0.0f, 200.0f ) );
   ECS::Assign<MotionComponent>( player );
   ECS::Assign<CameraComponent>( player );

   const EntityHandle sun = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( sun );
   ECS::Assign<LightComponent>( sun );

   for( uint32_t i = 0; i < 64; ++i )
   {
      const float x = ( ( ( i / 8 ) % 8 ) * 50.0f ) - ( 4 * 50.0f );
      const float y = ( ( i % 8 ) * 50.0f ) - ( 4 * 50.0f );

      const EntityHandle rock = ECS::CreateEntity();
      ECS::Assign<TransformComponent>( rock, glm::vec3( x, y, 0.0f ) );
      ECS::Assign<MeshComponent>( rock, "sphere" );
      ECS::Assign<RenderableComponent>( rock, StaticPipelines::Type::PBR, "PBR/layered-rock1" );
   }
}

void VKSandbox::tick( double deltaS )
{
   static uint32_t frames = 0;
   frames++;
   if( frames > 50 )
   {
      printf( "FPS: %f\n", 1.0 / deltaS );
      frames = 0;
   }

   GRIS::PrepareFrame();
   ECS::Tick( deltaS );
   GRIS::PresentFrame();
}

VKSandbox::~VKSandbox()
{
   // Core uninitializers
   ECS::Uninitialize();
   GRIS::UninitRenderBackend();
}
}
