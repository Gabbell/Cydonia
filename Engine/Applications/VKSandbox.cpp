#include <Applications/VKSandbox.h>

#include <Graphics/RenderInterface.h>

#include <ECS/ECS.h>

#include <ECS/Systems/RenderSystem.h>

#include <ECS/Components/TransformComponent.h>
#include <ECS/Components/RenderableComponent.h>

namespace cyd
{
VKSandbox::VKSandbox() = default;

bool VKSandbox::init( uint32_t width, uint32_t height, const std::string& title )
{
   Application::init( width, height, title );

   GRIS::InitRenderBackend<VK>( *m_window );
   ECS::Initialize();

   return true;
}

void VKSandbox::preLoop()
{
   ECS::AddSystem<RenderSystem>();

   const EntityHandle triangle = ECS::CreateEntity();
   ECS::Assign<TransformComponent>( triangle );
   ECS::Assign<RenderableComponent>( triangle );
}

void VKSandbox::tick( double deltaMs ) { ECS::Tick( deltaMs ); }

void VKSandbox::postLoop() {}

VKSandbox::~VKSandbox() { ECS::Uninitialize(); }
}
