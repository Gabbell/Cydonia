#include <VKSandbox.h>

#include <Graphics/AssetStash.h>
#include <Graphics/RenderGraph.h>
#include <Graphics/VertexLayout.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/MeshGeneration.h>

#include <ECS/Systems/Lighting/LightUpdateSystem.h>
#include <ECS/Systems/Input/InputSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MotionSystem.h>
#include <ECS/Systems/Procedural/FFTOceanSystem.h>
#include <ECS/Systems/Rendering/PBRRenderSystem.h>
#include <ECS/Systems/Resources/MaterialLoaderSystem.h>
#include <ECS/Systems/Resources/MeshLoaderSystem.h>
#include <ECS/Systems/Rendering/AtmosphereRenderSystem.h>
#include <ECS/Systems/Scene/CameraSystem.h>
#include <ECS/Systems/UI/ImGuiSystem.h>

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
   GRIS::InitRenderBackend( GRIS::API::VK, *m_window );
   GRIS::InitializeUI();

   m_ecs    = std::make_unique<EntityManager>();
   m_assets = std::make_unique<AssetStash>();
}

void VKSandbox::preLoop()
{
   // Systems Initialization
   // =============================================================================================

   // Core
   m_ecs->addSystem<InputSystem>( *m_window );
   m_ecs->addSystem<CameraSystem>();

   // Resources
   m_ecs->addSystem<MeshLoaderSystem>( *m_assets );
   m_ecs->addSystem<MaterialLoaderSystem>( *m_assets );

   // Physics/Motion
   m_ecs->addSystem<PlayerMoveSystem>();
   m_ecs->addSystem<MotionSystem>();

   // Rendering
   m_ecs->addSystem<LightUpdateSystem>();
   m_ecs->addSystem<FFTOceanSystem>();
   m_ecs->addSystem<PBRRenderSystem>();
   // m_ecs->addSystem<ImGuiSystem>();

   // Adding entities
   // =============================================================================================

   const EntityHandle player = m_ecs->createEntity();
   m_ecs->assign<InputComponent>( player );
   m_ecs->assign<TransformComponent>( player, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   m_ecs->assign<MotionComponent>( player );
   m_ecs->assign<CameraComponent>( player );

   const EntityHandle sun = m_ecs->createEntity();
   m_ecs->assign<RenderableComponent>( sun );
   m_ecs->assign<TransformComponent>( sun );
   m_ecs->assign<LightComponent>( sun );

   const EntityHandle ocean = m_ecs->createEntity();
   m_ecs->assign<RenderableComponent>( ocean );
   m_ecs->assign<TransformComponent>( ocean, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3(10.0f) );
   m_ecs->assign<MeshComponent>( ocean, "Ocean" );
   m_ecs->assign<FFTOceanComponent>( ocean, 256, 100, 10.0f, 40.0f, 1.0f, 0.0f );
   m_ecs->assign<MaterialComponent>( ocean, "OCEAN_RENDER" );

   std::vector<Vertex> gridVerts;
   std::vector<uint32_t> gridIndices;
   MeshGeneration::Grid( gridVerts, gridIndices, 512, 512 );

   // Creating grid mesh used for the ocean
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER, "Ocean Transfer" );
   GRIS::StartRecordingCommandList( transferList );
   m_assets->loadMesh( transferList, "Ocean", gridVerts, gridIndices );
   GRIS::EndRecordingCommandList( transferList );
   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
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

   m_ecs->tick( deltaS );

   RenderGraph::Execute();

   GRIS::PresentFrame();

   GRIS::RenderBackendCleanup();
}

VKSandbox::~VKSandbox()
{
   m_assets.reset();

   // Core uninitializers
   GRIS::UninitializeUI();
   GRIS::UninitRenderBackend();
}
}
