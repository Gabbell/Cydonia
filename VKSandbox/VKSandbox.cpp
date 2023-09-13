#include <VKSandbox.h>

#include <Graphics/StaticPipelines.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <Graphics/Utility/MeshGeneration.h>
#include <Graphics/Utility/Noise.h>

#include <ECS/Systems/Input/InputSystem.h>

#include <ECS/Systems/Debug/DebugDrawSystem.h>
#include <ECS/Systems/Lighting/LightUpdateSystem.h>
#include <ECS/Systems/Lighting/ShadowMapSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MotionSystem.h>
#include <ECS/Systems/Procedural/ProceduralDisplacementSystem.h>
#include <ECS/Systems/Procedural/FFTOceanSystem.h>
#include <ECS/Systems/Procedural/FogSystem.h>
#include <ECS/Systems/Procedural/AtmosphereSystem.h>
#include <ECS/Systems/Rendering/TessellationUpdateSystem.h>
#include <ECS/Systems/Rendering/ForwardRenderSystem.h>
#include <ECS/Systems/Resources/MaterialLoaderSystem.h>
#include <ECS/Systems/Resources/MeshLoaderSystem.h>
#include <ECS/Systems/Scene/ViewUpdateSystem.h>
#include <ECS/Systems/UI/ImGuiSystem.h>

#include <ECS/Components/Physics/MotionComponent.h>
#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>
#include <ECS/Components/Procedural/FFTOceanComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Rendering/FullscreenComponent.h>
#include <ECS/Components/Rendering/InstancedComponent.h>
#include <ECS/Components/Rendering/TessellatedComponent.h>

#if CYD_DEBUG
#include <ECS/Components/Debug/DebugDrawComponent.h>
#endif

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/EntityManager.h>

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
   m_ecs->addSystem<LightUpdateSystem>();
   m_ecs->addSystem<ViewUpdateSystem>();

   // Resources
   m_ecs->addSystem<MeshLoaderSystem>( *m_meshes );
   m_ecs->addSystem<MaterialLoaderSystem>( *m_materials );

   // Physics/Motion
   m_ecs->addSystem<PlayerMoveSystem>();
   m_ecs->addSystem<MotionSystem>();

   // Procedural
   m_ecs->addSystem<ProceduralDisplacementSystem>( *m_materials );
   m_ecs->addSystem<FFTOceanSystem>( *m_materials );

   // Rendering
   m_ecs->addSystem<TessellationUpdateSystem>();
   m_ecs->addSystem<ShadowMapSystem>( *m_materials );
   m_ecs->addSystem<ForwardRenderSystem>( *m_materials );
   m_ecs->addSystem<AtmosphereSystem>();

   // Debug
#if CYD_DEBUG
   m_ecs->addSystem<DebugDrawSystem>( *m_meshes );
#endif

   // UI
   m_ecs->addSystem<ImGuiSystem>( *m_ecs );

   // Creating terrain mesh
   // =============================================================================================
   CmdListHandle transferList = GRIS::CreateCommandList( QueueUsage::TRANSFER, "Initial Transfer" );

   std::vector<Vertex> vertices;
   std::vector<uint32_t> indices;
   MeshGeneration::PatchGrid( vertices, indices, 64 );
   m_meshes->loadMesh( transferList, "GRID", vertices, indices );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   // Adding entities
   // =============================================================================================
   const EntityHandle player = m_ecs->createEntity( "Player" );
   m_ecs->assign<InputComponent>( player );
   m_ecs->assign<TransformComponent>( player, glm::vec3( 0.0f, 15.0f, 0.0f ) );
   m_ecs->assign<MotionComponent>( player );
   m_ecs->assign<ViewComponent>( player, "MAIN" );

   const EntityHandle sun = m_ecs->createEntity( "Sun" );
   m_ecs->assign<TransformComponent>( sun, glm::vec3( 90.0f, 40.0f, 0.0f ) );
   m_ecs->assign<LightComponent>( sun );
   m_ecs->assign<ViewComponent>( sun, "SUN", -71.0f, 71.0f, -71.0f, 71.0f, -200.0f, 200.0f );

#if CYD_DEBUG
   m_ecs->assign<DebugDrawComponent>( sun, DebugDrawComponent::Type::SPHERE );
#endif

   //const EntityHandle terrain = m_ecs->createEntity( "Terrain" );
   //m_ecs->assign<RenderableComponent>( terrain, true, true );
   //m_ecs->assign<TransformComponent>( terrain, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 1.0f ) );
   //m_ecs->assign<MeshComponent>( terrain, "GRID" );
   //m_ecs->assign<TessellatedComponent>( terrain, 0.319f, 0.025f );
   //m_ecs->assign<MaterialComponent>( terrain, "TERRAIN", "TERRAIN_DISPLACEMENT" );
   //m_ecs->assign<ProceduralDisplacementComponent>(
   //    terrain, Noise::Type::SIMPLEX_NOISE, 2048, 2048, 0.0f );

   const EntityHandle ocean = m_ecs->createEntity( "Ocean" );
   m_ecs->assign<RenderableComponent>( ocean, false, true );
   m_ecs->assign<TransformComponent>( ocean, glm::vec3( 0.0f, 1.0f, 0.0f ), glm::vec3( 1.0f ) );
   m_ecs->assign<MeshComponent>( ocean, "GRID" );
   m_ecs->assign<TessellatedComponent>( ocean, 0.319f, 0.025f );
   m_ecs->assign<MaterialComponent>( ocean, "OCEAN", "OCEAN" );

   FFTOceanComponent::Description oceanDesc;
   oceanDesc.resolution          = 1024;
   oceanDesc.amplitude           = 62.0f;
   oceanDesc.horizontalDimension = 1000;
   oceanDesc.horizontalScale     = 12.0f;
   oceanDesc.verticalScale       = 17.0f;
   m_ecs->assign<FFTOceanComponent>( ocean, oceanDesc );

   const EntityHandle atmosphere = m_ecs->createEntity( "Atmosphere" );
   m_ecs->assign<RenderableComponent>( atmosphere );
   m_ecs->assign<AtmosphereComponent>( atmosphere );
}

void VKSandbox::tick( double deltaS )
{
   GRIS::BeginFrame();

   RenderGraph::Prepare();

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
