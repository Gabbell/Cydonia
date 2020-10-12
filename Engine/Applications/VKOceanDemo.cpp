#include <Applications/VKOceanDemo.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/Utility/MeshGeneration.h>

#include <ECS/EntityManager.h>

#include <ECS/Systems/Input/InputSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MovementSystem.h>
#include <ECS/Systems/Rendering/RenderSystem.h>
#include <ECS/Systems/Scene/CameraSystem.h>
#include <ECS/Systems/Lighting/LightSystem.h>
#include <ECS/Systems/Behaviour/EntityFollowSystem.h>
#include <ECS/Systems/Procedural/FFTOceanSystem.h>

#include <ECS/Components/Procedural/FFTOceanComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Transforms/TransformComponent.h>
#include <ECS/Components/Behaviour/EntityFollowComponent.h>
#include <ECS/Components/Rendering/SkyboxRenderableComponent.h>

#include <ECS/SharedComponents/InputComponent.h>

namespace CYD
{
static const std::vector<Vertex> skyboxVertices = {
    // FT
    {{{-250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f}},
     {{-250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, -1.0f}},
     {{250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}},
     {{250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}},
     {{250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f}},
     {{-250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f}},

     // LF
     {{-250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}},
     {{-250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, -1.0f}},
     {{-250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f}},
     {{-250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f}},
     {{-250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
     {{-250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}},

     // RT
     {{250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}},
     {{250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, 1.0f}},
     {{250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}},
     {{250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}},
     {{250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f}},
     {{250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}},

     // BK
     {{-250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}},
     {{-250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}},
     {{250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}},
     {{250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}},
     {{250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, 1.0f}},
     {{-250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}},

     // UP
     {{-250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}},
     {{250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, -1.0f}},
     {{250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}},
     {{250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, -1.0f, 1.0f}},
     {{-250.0f, 250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, 1.0f}},
     {{-250.0f, 250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, -1.0f, -1.0f}},

     // DN
     {{-250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, -1.0f}},
     {{-250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}},
     {{250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f}},
     {{250.0f, -250.0f, -250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, -1.0f}},
     {{-250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {-1.0f, 1.0f, 1.0f}},
     {{250.0f, -250.0f, 250.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}}};

VKOceanDemo::VKOceanDemo( uint32_t width, uint32_t height, const char* title )
    : Application( width, height, title )
{
   // Core initializers
   GRIS::InitRenderBackend<VK>( *m_window );
   ECS::Initialize();
}

void VKOceanDemo::preLoop()
{
   // This order is the order in which the systems will be ticked
   ECS::AddSystem<InputSystem>( *m_window );
   ECS::AddSystem<CameraSystem>();

   ECS::AddSystem<PlayerMoveSystem>();
   ECS::AddSystem<MovementSystem>();
   ECS::AddSystem<EntityFollowSystem>();

   ECS::AddSystem<LightSystem>();
   ECS::AddSystem<FFTOceanSystem>();

   ECS::AddSystem<RenderSystem>();

   // Creating player entity
   const EntityHandle player = ECS::CreateEntity();
   ECS::Assign<InputComponent>( player );
   ECS::Assign<TransformComponent>( player, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MotionComponent>( player );
   ECS::Assign<CameraComponent>( player );

   std::vector<Vertex> gridVerts;
   std::vector<uint32_t> gridIndices;
   MeshGen::Grid( 256, 256, gridVerts, gridIndices );

   const EntityHandle ocean = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( ocean, glm::vec3( 0.0f, 0.0f, 0.0f ) );
   ECS::Assign<MeshComponent>( ocean, gridVerts, gridIndices );
   ECS::Assign<FFTOceanComponent>( ocean, 256, 1000, 10.0f, 40.0f, 1.0f, 0.0f );
   ECS::Assign<RenderableComponent>( ocean );

   const EntityHandle skybox = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( skybox );
   ECS::Assign<MeshComponent>( skybox, skyboxVertices );
   ECS::Assign<EntityFollowComponent>( skybox, player );
   ECS::Assign<SkyboxRenderableComponent>( skybox, "MCLITE" );
}

void VKOceanDemo::tick( double deltaS )
{
   GRIS::PrepareFrame();
   ECS::Tick( deltaS );
   GRIS::PresentFrame();
}

VKOceanDemo::~VKOceanDemo()
{
   // Core uninitializers
   ECS::Uninitialize();
   GRIS::UninitRenderBackend();
}
}
