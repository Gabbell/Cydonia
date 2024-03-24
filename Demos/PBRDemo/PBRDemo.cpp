#include <PBRDemo.h>

#include <Graphics/StaticPipelines.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <Graphics/Utility/MeshGeneration.h>

#include <ECS/Components/Physics/MotionComponent.h>
#include <ECS/Components/Procedural/FFTOceanComponent.h>
#include <ECS/Components/Rendering/InstancedComponent.h>
#include <ECS/Components/Rendering/MaterialComponent.h>
#include <ECS/Components/Rendering/RenderableComponent.h>
#include <ECS/Components/Rendering/TessellatedComponent.h>
#include <ECS/Systems/Input/WindowSystem.h>
#include <ECS/Systems/Lighting/LightUpdateSystem.h>
#include <ECS/Systems/Lighting/ShadowMapSystem.h>
#include <ECS/Systems/Lighting/ShadowMapUpdateSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MotionSystem.h>
#include <ECS/Systems/Procedural/FFTOceanUpdateSystem.h>
#include <ECS/Systems/Procedural/AtmosphereUpdateSystem.h>
#include <ECS/Systems/Rendering/TessellationUpdateSystem.h>
#include <ECS/Systems/Rendering/GBufferSystem.h>
#include <ECS/Systems/Rendering/DeferredRenderSystem.h>
#include <ECS/Systems/Rendering/ForwardRenderSystem.h>
#include <ECS/Systems/Rendering/AtmosphereSystem.h>
#include <ECS/Systems/Resources/PipelineLoaderSystem.h>
#include <ECS/Systems/Resources/MaterialLoaderSystem.h>
#include <ECS/Systems/Resources/MeshLoaderSystem.h>
#include <ECS/Systems/Scene/ViewUpdateSystem.h>
#include <ECS/Systems/UI/ImGuiSystem.h>

#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>
#include <ECS/EntityManager.h>

#include <UI/UserInterface.h>

#include <Profiling.h>

#include <cstdlib>

namespace CYD
{
PBRDemo::PBRDemo( uint32_t width, uint32_t height, const char* title )
    : Application( width, height, title )
{
   // Core initializers
   GRIS::InitRenderBackend( GRIS::API::VK, *m_window );
   StaticPipelines::Initialize();

   m_meshes    = std::make_unique<MeshCache>( *m_threadPool );
   m_materials = std::make_unique<MaterialCache>( *m_threadPool );
   m_ecs       = std::make_unique<EntityManager>();
}

void PBRDemo::preLoop()
{
   // Systems Initialization
   // =============================================================================================

   // Core
   m_ecs->addSystem<WindowSystem>( *m_window );
   m_ecs->addSystem<PlayerMoveSystem>();

   // Physics/Motion
   m_ecs->addSystem<MotionSystem>();

   // Update
   m_ecs->addSystem<LightUpdateSystem>();
   m_ecs->addSystem<ViewUpdateSystem>();
   m_ecs->addSystem<ShadowMapUpdateSystem>();
   m_ecs->addSystem<TessellationUpdateSystem>();

   // Resources
   m_ecs->addSystem<PipelineLoaderSystem>();
   m_ecs->addSystem<MeshLoaderSystem>( *m_meshes );
   m_ecs->addSystem<MaterialLoaderSystem>( *m_materials );

   // Pre-Render
   m_ecs->addSystem<FFTOceanUpdateSystem>( *m_materials );
   m_ecs->addSystem<AtmosphereUpdateSystem>();

   m_ecs->addSystem<ShadowMapSystem>( *m_meshes, *m_materials );
   m_ecs->addSystem<GBufferSystem>( *m_meshes, *m_materials );

   // Rendering
   m_ecs->addSystem<DeferredRenderSystem>();
   m_ecs->addSystem<ForwardRenderSystem>( *m_meshes, *m_materials );

   // Post-Process
   m_ecs->addSystem<AtmosphereSystem>();

   // UI
   m_ecs->addSystem<ImGuiSystem>( *m_ecs );

   // Adding entities
   // =============================================================================================
   const EntityHandle player = m_ecs->createEntity( "Player" );
   m_ecs->assign<InputComponent>( player );
   m_ecs->assign<TransformComponent>( player, glm::vec3( 0.0f, 50.0f, -50.0f ) );
   m_ecs->assign<MotionComponent>( player, 100.0f, 10.0f );
   m_ecs->assign<ViewComponent>( player, 60.0f /*fov*/, 0.1f /*near*/, 32000.0f /*far*/ );

   const EntityHandle sun = m_ecs->createEntity( "Sun" );
   m_ecs->assign<TransformComponent>( sun );
   m_ecs->assign<LightComponent>(
       sun, LightComponent::Type::DIRECTIONAL, glm::vec4( 1.0f ), glm::vec3( 0.0f, -0.5f, 1.0f ) );
   m_ecs->assign<ShadowMapComponent>( sun );
   m_ecs->assign<ViewComponent>( sun, player /*fitToEntity*/ );

   RenderableComponent::Description swordDesc;
   swordDesc.type              = RenderableComponent::Type::DEFERRED;
   swordDesc.pipelineName      = "PBR_GBUFFER";
   swordDesc.isShadowReceiving = true;

   const EntityHandle sword = m_ecs->createEntity( "Sword" );
   m_ecs->assign<RenderableComponent>( sword, swordDesc );
   m_ecs->assign<TransformComponent>( sword, glm::vec3( 0.0f, 50.0f, 0.0f ), glm::vec3( 1.0f ) );
   m_ecs->assign<MeshComponent>( sword, "Sword" );
   m_ecs->assign<MaterialComponent>( sword, "SWORD" );

   // Atmosphere
   AtmosphereComponent::Description atmosDesc;
   atmosDesc.nearClip                      = 0.1f;
   atmosDesc.farClip                       = 32000.0f;
   atmosDesc.mieScatteringCoefficient      = glm::vec3( 0.576f, 0.576f, 0.576f );
   atmosDesc.mieAbsorptionCoefficient      = glm::vec3( 0.576f, 0.576f, 0.576f );
   atmosDesc.rayleighScatteringCoefficient = glm::vec3( 0.161f, 0.373f, 0.914f );
   atmosDesc.absorptionCoefficient         = glm::vec3( 0.325f, 0.945f, 0.043f );
   atmosDesc.groundAlbedo                  = glm::vec3( 0.0f );
   atmosDesc.groundRadiusMM                = 6.36f;
   atmosDesc.atmosphereRadiusMM            = 6.46f;
   atmosDesc.miePhase                      = 0.8f;
   atmosDesc.mieScatteringScale            = 0.00952f;
   atmosDesc.mieAbsorptionScale            = 0.00077f;
   atmosDesc.rayleighScatteringScale       = 0.03624f;
   atmosDesc.absorptionScale               = 0.00199f;
   atmosDesc.rayleighHeight                = 8.0f;
   atmosDesc.mieHeight                     = 1.2f;
   atmosDesc.heightFogHeight               = 0.025f;
   atmosDesc.heightFogFalloff              = 0.016f;
   atmosDesc.heightFogStrength             = 0.0f;

   const EntityHandle atmosphere = m_ecs->createEntity( "Atmosphere" );
   m_ecs->assign<RenderableComponent>( atmosphere );
   m_ecs->assign<AtmosphereComponent>( atmosphere, atmosDesc );
}

void PBRDemo::tick( double deltaS )
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

PBRDemo::~PBRDemo()
{
   GRIS::WaitUntilIdle();

   m_ecs.reset();
   m_materials.reset();
   m_meshes.reset();

   // Core uninitializers
   GRIS::UninitRenderBackend();
   StaticPipelines::Uninitialize();
}
}
