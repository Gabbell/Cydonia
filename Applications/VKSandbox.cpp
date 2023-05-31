#include <VKSandbox.h>

#include <Graphics/GRIS/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Scene/MeshCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <ECS/Systems/Input/InputSystem.h>
#include <ECS/Systems/Rendering/FullscreenRenderSystem.h>
#include <ECS/Systems/Debug/DebugDrawSystem.h>
#include <ECS/Systems/Resources/MaterialLoaderSystem.h>
#include <ECS/Systems/Resources/MeshLoaderSystem.h>
#include <ECS/Systems/Procedural/NoiseGenerationSystem.h>
#include <ECS/Systems/Scene/CameraSystem.h>
#include <ECS/Systems/UI/ImGuiSystem.h>

#include <ECS/Components/Rendering/FullscreenComponent.h>
#include <ECS/Components/Procedural/NoiseComponent.h>

#if CYD_DEBUG
#include <ECS/Components/Debug/DebugSphereComponent.h>
#endif

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

   StaticPipelines::Initialize();

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

   // Resources
   m_ecs->addSystem<MeshLoaderSystem>( *m_meshes );
   m_ecs->addSystem<MaterialLoaderSystem>( *m_materials );

   // Procedural
   m_ecs->addSystem<NoiseGenerationSystem>( *m_materials );

   // Rendering
   m_ecs->addSystem<FullscreenRenderSystem>( *m_materials );

   // Debug
#if CYD_DEBUG
   // m_ecs->addSystem<DebugDrawSystem>( *m_meshes );
#endif

   // UI
   // m_ecs->addSystem<ImGuiSystem>();

   // Adding entities
   // =============================================================================================
   const EntityHandle noiseTexture = m_ecs->createEntity();
   m_ecs->assign<FullscreenComponent>( noiseTexture );
   m_ecs->assign<NoiseComponent>( noiseTexture );
   m_ecs->assign<MaterialComponent>( noiseTexture, "SIMPLEX_NOISE", "SOLO_TEX" );
}

void VKSandbox::tick( double deltaS )
{
   static double timeAccum    = 0;
   static uint32_t fpsAccum   = 0;
   static uint32_t frameAccum = 0;

   timeAccum += deltaS;
   fpsAccum += static_cast<uint32_t>( 1.0 / deltaS );
   frameAccum++;

   if( timeAccum > 1.0 )
   {
      printf( "Average FPS: %d\n", fpsAccum / frameAccum );

      timeAccum  = 0.0;
      fpsAccum   = 0;
      frameAccum = 0;
   }

   GRIS::PrepareFrame();

   m_ecs->tick( deltaS );

   RenderGraph::Execute();

   GRIS::PresentFrame();

   GRIS::RenderBackendCleanup();
}

VKSandbox::~VKSandbox()
{
   m_ecs.reset();
   m_materials.reset();
   m_meshes.reset();

   // Core uninitializers
   GRIS::UninitializeUI();
   GRIS::UninitRenderBackend();
   StaticPipelines::Uninitialize();
}
}
