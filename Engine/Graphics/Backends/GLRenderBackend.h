#pragma once

#include <Graphics/Backends/RenderBackend.h>

#include <Common/Include.h>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Window;
class GLRenderBackendImp;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
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