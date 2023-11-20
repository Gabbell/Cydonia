#include <Sandbox.h>

#include <Graphics/StaticPipelines.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/TextureCache.h>

#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <Graphics/Utility/MeshGeneration.h>
#include <Graphics/Utility/Noise.h>

#include <ECS/Components/Physics/MotionComponent.h>
#include <ECS/Components/Procedural/FFTOceanComponent.h>
#include <ECS/Components/Procedural/ProceduralDisplacementComponent.h>
#include <ECS/Components/Rendering/InstancedComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/TessellatedComponent.h>
#include <ECS/Systems/Debug/DebugDrawSystem.h>
#include <ECS/Systems/Input/WindowSystem.h>
#include <ECS/Systems/Lighting/LightUpdateSystem.h>
#include <ECS/Systems/Lighting/ShadowMapSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MotionSystem.h>
#include <ECS/Systems/Procedural/ProceduralDisplacementSystem.h>
#include <ECS/Systems/Procedural/AtmosphereSystem.h>
#include <ECS/Systems/Rendering/TessellationUpdateSystem.h>
#include <ECS/Systems/Rendering/ForwardRenderSystem.h>
#include <ECS/Systems/Rendering/GBufferSystem.h>
#include <ECS/Systems/Rendering/DeferredRenderSystem.h>
#include <ECS/Systems/Rendering/AtmosphereRenderSystem.h>
#include <ECS/Systems/Resources/PipelineLoaderSystem.h>
#include <ECS/Systems/Resources/MaterialLoaderSystem.h>
#include <ECS/Systems/Resources/MeshLoaderSystem.h>
#include <ECS/Systems/Scene/ViewUpdateSystem.h>
#include <ECS/Systems/UI/ImGuiSystem.h>

#if CYD_DEBUG
#include <ECS/Components/Debug/DebugDrawComponent.h>
#endif

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>
#include <ECS/EntityManager.h>

#include <UI/UserInterface.h>

#include <Profiling.h>

#include <cstdlib>

namespace CYD
{
Sandbox::Sandbox( uint32_t width, uint32_t height, const char* title )
    : Application( width, height, title )
{
   // Core initializers
   GRIS::InitRenderBackend( GRIS::API::VK, *m_window );
   GRIS::TextureCache::Initialize();
   StaticPipelines::Initialize();
   Noise::Initialize();

   m_meshes    = std::make_unique<MeshCache>( *m_threadPool );
   m_materials = std::make_unique<MaterialCache>( *m_threadPool );
   m_ecs       = std::make_unique<EntityManager>();
}

void Sandbox::preLoop()
{
   // Systems Initialization
   // =============================================================================================

   // Core
   m_ecs->addSystem<WindowSystem>( *m_window );
   m_ecs->addSystem<LightUpdateSystem>();
   m_ecs->addSystem<ViewUpdateSystem>();
   m_ecs->addSystem<TessellationUpdateSystem>();

   // Resources
   m_ecs->addSystem<PipelineLoaderSystem>();
   m_ecs->addSystem<MeshLoaderSystem>( *m_meshes );
   m_ecs->addSystem<MaterialLoaderSystem>( *m_materials );

   // Physics/Motion
   m_ecs->addSystem<PlayerMoveSystem>();
   m_ecs->addSystem<MotionSystem>();

   // Pre-Render
   m_ecs->addSystem<ProceduralDisplacementSystem>( *m_materials );
   m_ecs->addSystem<AtmosphereSystem>();
   m_ecs->addSystem<ShadowMapSystem>( *m_meshes, *m_materials );
   m_ecs->addSystem<GBufferSystem>( *m_meshes, *m_materials );

   // Rendering
   m_ecs->addSystem<DeferredRenderSystem>();
   m_ecs->addSystem<ForwardRenderSystem>( *m_meshes, *m_materials );

   // Post-Process
   m_ecs->addSystem<AtmosphereRenderSystem>();

   // Debug
#if CYD_DEBUG
   m_ecs->addSystem<DebugDrawSystem>( *m_meshes );
#endif

   // UI
   m_ecs->addSystem<ImGuiSystem>( *m_ecs );

   // Adding entities
   // =============================================================================================
   const EntityHandle player = m_ecs->createEntity( "Player" );
   m_ecs->assign<InputComponent>( player );
   m_ecs->assign<TransformComponent>( player, glm::vec3( 0.0f, 800.0f, 3000.0f ) );
   m_ecs->assign<MotionComponent>( player, 1000.0f, 100.0f );
   m_ecs->assign<ViewComponent>( player, "MAIN" );

   const EntityHandle sun = m_ecs->createEntity( "Sun" );
   m_ecs->assign<TransformComponent>( sun, glm::vec3( 0.0f, 20.0f, -90.0f ) );
   m_ecs->assign<LightComponent>( sun );
   m_ecs->assign<ViewComponent>(
       sun, "SUN", -3600.0f, 3600.0f, -3600.0f, 3600.0f, -5000.0f, 5000.0f );

#if CYD_DEBUG
   m_ecs->assign<DebugDrawComponent>( sun, DebugDrawComponent::Type::SPHERE );
#endif

   // Terrain
   RenderableComponent::Description terrainDesc;
   terrainDesc.pipelineName      = "TERRAIN_GBUFFER";
   terrainDesc.type              = RenderableComponent::Type::DEFERRED;
   terrainDesc.isShadowCasting   = true;
   terrainDesc.isShadowReceiving = true;

   Noise::ShaderParams terrainNoise;
   terrainNoise.gain       = 0.318f;
   terrainNoise.frequency  = 2.609f;
   terrainNoise.lacunarity = 3.103f;
   terrainNoise.ridged     = true;
   terrainNoise.invert     = true;
   terrainNoise.octaves    = 5;

   const EntityHandle terrain = m_ecs->createEntity( "Terrain" );
   m_ecs->assign<RenderableComponent>( terrain, terrainDesc );
   m_ecs->assign<TransformComponent>( terrain, glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( 50.0f ) );
   m_ecs->assign<MeshComponent>( terrain, "GRID" );
   m_ecs->assign<TessellatedComponent>( terrain, 0.04f, 0.85f );
   m_ecs->assign<MaterialComponent>( terrain, "TERRAIN_DISPLACEMENT" );
   m_ecs->assign<ProceduralDisplacementComponent>(
       terrain, Noise::Type::SIMPLEX_NOISE, 2048, 2048, terrainNoise );

   // Atmosphere
   AtmosphereComponent::Description atmosDesc;
   atmosDesc.mieScatteringCoefficient      = glm::vec3( 0.576f, 0.576f, 0.576f );
   atmosDesc.mieAbsorptionCoefficient      = glm::vec3( 0.576f, 0.576f, 0.576f );
   atmosDesc.rayleighScatteringCoefficient = glm::vec3( 0.161f, 0.373f, 0.914f );
   atmosDesc.absorptionCoefficient         = glm::vec3( 0.325f, 0.945f, 0.043f );
   atmosDesc.groundAlbedo                  = glm::vec3( 0.0f );
   atmosDesc.groundRadiusMM                = 6.36f;
   atmosDesc.atmosphereRadiusMM            = 6.46f;
   atmosDesc.miePhase                      = 0.92f;
   atmosDesc.mieScatteringScale            = 0.00952f;
   atmosDesc.mieAbsorptionScale            = 0.00077f;
   atmosDesc.rayleighScatteringScale       = 0.03624f;
   atmosDesc.absorptionScale               = 0.00199f;
   atmosDesc.rayleighHeight                = 8.0f;
   atmosDesc.mieHeight                     = 1.2f;
   atmosDesc.heightFogHeight               = 0.01268f;
   atmosDesc.heightFogFalloff              = 0.01857f;
   atmosDesc.heightFogStrength             = 0.5f;

   const EntityHandle atmosphere = m_ecs->createEntity( "Atmosphere" );
   m_ecs->assign<RenderableComponent>( atmosphere );
   m_ecs->assign<AtmosphereComponent>( atmosphere, atmosDesc );
}

void Sandbox::tick( double deltaS )
{
   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   GRIS::BeginFrame();

   RenderGraph::Prepare();

   scene.mainFramebuffer.setClearAll( true );

   m_ecs->tick( deltaS );

   // Copy main color to swapchain
   const CmdListHandle cmdList = RenderGraph::GetCommandList( RenderGraph::Pass::LAST );
   GRIS::CopyToSwapchain( cmdList, scene.mainColor );

   RenderGraph::Execute();

   GRIS::PresentFrame();

   GRIS::RenderBackendCleanup();
}

Sandbox::~Sandbox()
{
   GRIS::WaitUntilIdle();

   m_ecs.reset();
   m_materials.reset();
   m_meshes.reset();

   // Core uninitializers
   GRIS::TextureCache::Uninitialize();
   GRIS::UninitRenderBackend();
   Noise::Uninitialize();
   StaticPipelines::Uninitialize();
}
}
