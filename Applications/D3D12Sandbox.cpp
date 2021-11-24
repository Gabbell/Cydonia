#include <D3D12Sandbox.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
D3D12Sandbox::D3D12Sandbox( uint32_t width, uint32_t height, const char* title )
    : Application( width, height, title )
{
   // Core initializers
   GRIS::InitRenderBackend( GRIS::API::D3D12, *m_window );
   GRIS::InitializeUI();
}

void D3D12Sandbox::preLoop() {}

void D3D12Sandbox::tick( double deltaS )
{
   static uint32_t frames = 0;
   frames++;
   if( frames > 50 )
   {
      printf( "FPS: %f\n", 1.0 / deltaS );
      frames = 0;
   }

   GRIS::PrepareFrame();

   GRIS::PresentFrame();

   GRIS::RenderBackendCleanup();
}

D3D12Sandbox::~D3D12Sandbox()
{
   // Core uninitializers
   GRIS::UninitializeUI();
   GRIS::UninitRenderBackend();
}
}
