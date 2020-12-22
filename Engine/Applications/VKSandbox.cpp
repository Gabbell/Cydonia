#include <Applications/VKSandbox.h>

#include <Graphics/AssetStash.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/Systems/Behaviour/EntityFollowSystem.h>
#include <ECS/Systems/Lighting/LightUpdateSystem.h>
#include <ECS/Systems/Input/InputSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MotionSystem.h>
#include <ECS/Systems/Rendering/PBRRenderSystem.h>
#include <ECS/Systems/Resources/MaterialLoaderSystem.h>
#include <ECS/Systems/Resources/MeshLoaderSystem.h>
#include <ECS/Systems/Lighting/ShadowMapSystem.h>
#include <ECS/Systems/Rendering/SkyboxSystem.h>
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
   GRIS::InitRenderBackend( GRIS::VK, *m_window );
   ECS::Initialize();

   m_assets = std::make_unique<AssetStash>();
}

void VKSandbox::preLoop()
{
   // TODO Better system scheduler

   // Systems Initialization
   // =============================================================================================

   // Core
   ECS::AddSystem<InputSystem>( *m_window );
   ECS::AddSystem<CameraSystem>();

   // Resources
   ECS::AddSystem<MeshLoaderSystem>( *m_assets );
   ECS::AddSystem<MaterialLoaderSystem>( *m_assets );

   // Physics/Motion
   ECS::AddSystem<PlayerMoveSystem>();
   ECS::AddSystem<MotionSystem>();

   // Behaviour
   ECS::AddSystem<EntityFollowSystem>();

   // Rendering
   ECS::AddSystem<LightUpdateSystem>();
   ECS::AddSystem<ShadowMapSystem>();
   ECS::AddSystem<SkyboxSystem>();
   ECS::AddSystem<PBRRenderSystem>();

   // Adding entities
   // =============================================================================================

   const EntityHandle player = ECS::CreateEntity();
   ECS::Assign<InputComponent>( player );
   ECS::Assign<TransformComponent>( player, glm::vec3( 0.0f, 0.0f, 200.0f ) );
   ECS::Assign<MotionComponent>( player );
   ECS::Assign<CameraComponent>( player );

   // Light entity
   const EntityHandle sun = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( sun );
   ECS::Assign<LightComponent>( sun, LightComponent::Type::DIRECTIONAL );

   // Some renderable entities
   for( uint32_t i = 0; i < 64; ++i )
   {
      const float x = ( ( ( i / 8 ) % 8 ) * 50.0f ) - ( 4 * 50.0f );
      const float y = ( ( i % 8 ) * 50.0f ) - ( 4 * 50.0f );

      const EntityHandle rock = ECS::CreateEntity();
      ECS::Assign<TransformComponent>( rock, glm::vec3( x, y, 0.0f ) );
      ECS::Assign<MeshComponent>( rock, "sphere" );
      ECS::Assign<MaterialComponent>( rock, "PBR", "PBR/layered-rock1" );
      ECS::Assign<RenderableComponent>( rock, true /*isOccluder*/ );
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

   ECS::Tick( deltaS );
}

VKSandbox::~VKSandbox()
{
   // Core uninitializers
   ECS::Uninitialize();
   GRIS::UninitRenderBackend();
}
}
