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
Ocean demo to show off the FFT Ocean
*/
namespace CYD
{
class OceanDemo final : public Application
{
  public:
   OceanDemo( uint32_t width, uint32_t height, const char* title );
   NON_COPIABLE( OceanDemo );
   ~OceanDemo() override;

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
