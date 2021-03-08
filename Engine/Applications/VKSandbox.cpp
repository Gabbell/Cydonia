#include <Applications/VKSandbox.h>

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
   GRIS::InitRenderBackend( GRIS::VK, *m_window );
   GRIS::InitializeUI();
   ECS::Initialize();

   m_assets = std::make_unique<AssetStash>();
}

void VKSandbox::preLoop()
{
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

   // Procedural
   ECS::AddSystem<FFTOceanSystem>();

   // Rendering
   ECS::AddSystem<LightUpdateSystem>();
   ECS::AddSystem<PBRRenderSystem>();
   // ECS::AddSystem<ImGuiSystem>();

   // Adding entities
   // =============================================================================================

   const EntityHandle player = ECS::CreateEntity();
   ECS::Assign<InputComponent>( player );
   ECS::Assign<TransformComponent>( player, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MotionComponent>( player );
   ECS::Assign<CameraComponent>( player );

   const EntityHandle sun = ECS::CreateEntity();
   ECS::Assign<RenderableComponent>( sun );
   ECS::Assign<TransformComponent>( sun );
   ECS::Assign<LightComponent>( sun );
   // ECS::Assign<MeshComponent>( sun, "sphere" );
   // ECS::Assign<MaterialComponent>( sun, "WIREFRAME" );

   const EntityHandle ocean = ECS::CreateEntity();
   ECS::Assign<RenderableComponent>( ocean );
   ECS::Assign<TransformComponent>(
       ocean, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 20.0f, 20.0f, 20.0f ) );
   ECS::Assign<MeshComponent>( ocean, "Ocean" );
   ECS::Assign<FFTOceanComponent>( ocean, 512, 150, 100.0f, 20.0f, 100.0f, 0.0f );
   ECS::Assign<MaterialComponent>( ocean, "OCEAN_RENDER" );

   std::vector<Vertex> gridVerts;
   std::vector<uint32_t> gridIndices;
   MeshGeneration::Grid( 512, 512, gridVerts, gridIndices );

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

   ECS::Tick( deltaS );

   RenderGraph::Execute();

   GRIS::PresentFrame();

   GRIS::RenderBackendCleanup();
}

VKSandbox::~VKSandbox()
{
   m_assets.reset();

   // Core uninitializers
   ECS::Uninitialize();
   GRIS::UninitializeUI();
   GRIS::UninitRenderBackend();
}
}
