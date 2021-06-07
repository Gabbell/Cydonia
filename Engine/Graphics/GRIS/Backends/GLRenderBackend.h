#pragma once

#include <Graphics/GRIS/Backends/RenderBackend.h>

#include <Common/Include.h>

// ================================================================================================
// Forwards
// ================================================================================================
namespace CYD
{
class Window;
class GLRenderBackendImp;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace CYD
{
class GLRenderBackend final : public RenderBackend
{
  public:
   GLRenderBackend( const Window& window );
   NON_COPIABLE( GLRenderBackend );
   virtual ~GLRenderBackend();

   void cleanup() override;

  private:
   GLRenderBackendImp* _imp;
};
}