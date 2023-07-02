#include <VKSandbox.h>

#include <Graphics/VertexLayout.h>
#include <Graphics/StaticPipelines.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <Graphics/Utility/MeshGeneration.h>
#include <Graphics/Utility/Noise.h>

#include <ECS/Systems/Input/InputSystem.h>
#include <ECS/Systems/Lighting/LightUpdateSystem.h>
#include <ECS/Systems/Lighting/ShadowMapSystem.h>
#include <ECS/Systems/Rendering/ForwardRenderSystem.h>
#include <ECS/Systems/Debug/DebugDrawSystem.h>
#include <ECS/Systems/Resources/MaterialLoaderSystem.h>
#include <ECS/Systems/Resources/MeshLoaderSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MotionSystem.h>
#include <ECS/Systems/Procedural/ProceduralDisplacementSystem.h>
#include <ECS/Systems/Scene/CameraSystem.h>
#include <ECS/Systems/Scene/InstanceUpdateSystem.h>
#include <ECS/Systems/UI/ImGuiSystem.h>

#include <ECS/Components/Physics/MotionComponent.h>
#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Transforms/InstancedComponent.h>

#if CYD_DEBUG
#include <ECS/Components/Debug/DebugDrawComponent.h>
#endif

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/EntityManager.h>

#include <Window/GLFWWindow.h>

#include <Profiling.h>

#include <cstdlib>

namespace CYD
{
VKSandbox::VKSandbox( uint32_t width, uint32_t height, const char* title )
    : Application( width, height, title )
{
   // Core initializers
   GRIS::InitRenderBackend( GRIS::API::VK, *m_window );

   StaticPipelines::Initialize();
   Noise::Initialize();

   m_meshes    = std::make_unique<MeshCache>();
   m_materials = std::make_unique<MaterialCache>();
   m_ecs       = std::make_unique<EntityManager>();
}

void VKSandbox::preLoop()
{
   // Systems Initialization
   // =============================================================================================
   // Core
   m_ecs->addSystem<InputSystem>( *m_window );
   m_ecs->addSystem<CameraSystem>();
   m_ecs->addSystem<InstanceUpdateSystem>();

   // Resources
   m_ecs->addSystem<MeshLoaderSystem>( *m_meshes );
   m_ecs->addSystem<MaterialLoaderSystem>( *m_materials );

   // Physics/Motion
   m_ecs->addSystem<PlayerMoveSystem>();
   m_ecs->addSystem<MotionSystem>();

   // Procedural
   m_ecs->addSystem<ProceduralDisplacementSystem>( *m_materials );

   // Rendering
   m_ecs->addSystem<LightUpdateSystem>();
   m_ecs->addSystem<ShadowMapSystem>( *m_materials );
   m_ecs->addSystem<ForwardRenderSystem>( *m_materials );

   // Debug
#if CYD_DEBUG
   m_ecs->addSystem<DebugDrawSystem>( *m_meshes );
#endif

   // UI
   m_ecs->addSystem<ImGuiSystem>( *m_ecs );

   // Creating terrain mesh and debug sphere
   // =============================================================================================
   CmdListHandle transferList = GRIS::CreateCommandList( QueueUsage::TRANSFER, "Initial Transfer" );

   std::vector<Vertex> terrainGridVerts;
   std::vector<uint32_t> terrainGridIndices;
   MeshGeneration::Grid( terrainGridVerts, terrainGridIndices, 1024, 1024 );
   m_meshes->loadMesh( transferList, "TERRAIN", terrainGridVerts, terrainGridIndices );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   // Adding entities
   // =============================================================================================
   const EntityHandle player = m_ecs->createEntity( "Player" );
   m_ecs->assign<InputComponent>( player );
   m_ecs->assign<TransformComponent>( player, glm::vec3( 0.0f, 15.0f, 0.0f ) );
   m_ecs->assign<MotionComponent>( player );
   m_ecs->assign<CameraComponent>( player, "MAIN" );

   const EntityHandle sun = m_ecs->createEntity( "Sun" );
   m_ecs->assign<TransformComponent>( sun, glm::vec3( 0.0f, 40.0f, 0.0f ) );
   m_ecs->assign<LightComponent>( sun );

#if CYD_DEBUG
   m_ecs->assign<DebugDrawComponent>( sun, DebugDrawComponent::Type::SPHERE );
#endif

   const EntityHandle terrain = m_ecs->createEntity( "Terrain" );
   m_ecs->assign<RenderableComponent>( terrain );
   m_ecs->assign<TransformComponent>( terrain, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 100.0f ) );
   m_ecs->assign<MeshComponent>( terrain, "TERRAIN" );
   m_ecs->assign<ProceduralDisplacementComponent>(
       terrain, Noise::Type::SIMPLEX_NOISE, 1024, 1024, 0.0f );
   m_ecs->assign<MaterialComponent>( terrain, "TERRAIN", "SOLO_TEX_R32F" );

   // const EntityHandle spheres = m_ecs->createEntity( "Spheres" );
   // m_ecs->assign<RenderableComponent>( spheres );
   // m_ecs->assign<TransformComponent>( spheres );
   // m_ecs->assign<InstancedComponent>( spheres, InstancedComponent::MAX_INSTANCES );
   // m_ecs->assign<MeshComponent>( spheres, "DEBUG_SPHERE" );
   // m_ecs->assign<MaterialComponent>( spheres, "PBR_CONSTANT", "NO_MATERIAL" );
}

void VKSandbox::tick( double deltaS )
{
   GRIS::PrepareFrame();

   m_ecs->tick( deltaS );

   RenderGraph::Execute();

   GRIS::PresentFrame();

   GRIS::RenderBackendCleanup();
}

VKSandbox::~VKSandbox()
{
   GRIS::WaitUntilIdle();

   m_ecs.reset();
   m_materials.reset();
   m_meshes.reset();

   // Core uninitializers
   GRIS::UninitRenderBackend();
   Noise::Uninitialize();
   StaticPipelines::Uninitialize();
}
}
