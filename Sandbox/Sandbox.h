#pragma once

#include <Application.h>

#include <Common/Include.h>

#include <Graphics/Framebuffer.h>

#include <memory>

// ================================================================================================
// Forwards
// ================================================================================================
namespace CYD
{
class EntityManager;
class MeshCache;
class MaterialCache;
}

// ================================================================================================
// Definition
// ================================================================================================
/*
This Vulkan sandbox is used as a template to experiment
*/
namespace CYD
{
class Sandbox final : public Application
{
  public:
   Sandbox( uint32_t width, uint32_t height, const char* title );
   NON_COPIABLE( Sandbox );
   ~Sandbox() override;

  protected:
   void preLoop() override;
   void tick( double deltaS ) override;

  private:
   // ECS and Caches
   std::unique_ptr<EntityManager> m_ecs;
   std::unique_ptr<MeshCache> m_meshes;
   std::unique_ptr<MaterialCache> m_materials;
};
}
