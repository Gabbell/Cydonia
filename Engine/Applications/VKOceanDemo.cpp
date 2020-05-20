#include <Applications/VKOceanDemo.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/MeshGeneration.h>

#include <ECS/EntityManager.h>

#include <ECS/Systems/Input/InputSystem.h>
#include <ECS/Systems/Physics/PlayerMoveSystem.h>
#include <ECS/Systems/Physics/MovementSystem.h>
#include <ECS/Systems/Procedural/FFTOceanSystem.h>
#include <ECS/Systems/Rendering/RenderSystem.h>
#include <ECS/Systems/Scene/CameraSystem.h>
#include <ECS/Systems/Scene/LightSystem.h>

#include <ECS/Components/Procedural/FFTOceanComponent.h>
#include <ECS/Components/Rendering/MeshComponent.h>
#include <ECS/Components/Rendering/PhongRenderableComponent.h>
#include <ECS/Components/Transforms/TransformComponent.h>

#include <ECS/SharedComponents/InputComponent.h>

namespace CYD
{
VKOceanDemo::VKOceanDemo( uint32_t width, uint32_t height, const std::string& title )
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
   ECS::AddSystem<PlayerMoveSystem>();
   ECS::AddSystem<MovementSystem>();
   ECS::AddSystem<CameraSystem>();
   ECS::AddSystem<LightSystem>();
   ECS::AddSystem<FFTOceanSystem>();
   ECS::AddSystem<RenderSystem>();

   std::vector<Vertex> gridVerts;
   std::vector<uint32_t> gridIndices;
   MeshGen::Grid( 256, 256, gridVerts, gridIndices );

   const EntityHandle ocean = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( ocean, glm::vec3( 0.0f, -10.0f, 0.0f ) );
   ECS::Assign<MeshComponent>( ocean, gridVerts, gridIndices );
   ECS::Assign<FFTOceanComponent>( ocean, 256, 1000, 4, 40.0f, 1.0f, 1.0f );
   ECS::Assign<PhongRenderableComponent>( ocean );
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
