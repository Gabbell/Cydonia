#include <Graphics/Backends/GLRenderBackend.h>

#include <Graphics/GraphicsTypes.h>

#include <Window/GLFWWindow.h>

namespace cyd
{
// =================================================================================================
// Implementation
class GLRenderBackendImp
{
  public:
   GLRenderBackendImp( const Window& ) { printf( "Initializing GL\n" ); }
   ~GLRenderBackendImp() {}

   void cleanup() {}
};

// =================================================================================================
// Redirections
GLRenderBackend::GLRenderBackend( const Window& window ) : _imp( new GLRenderBackendImp( window ) )
{
}
GLRenderBackend::~GLRenderBackend() { delete _imp; }
void GLRenderBackend::cleanup() { _imp->cleanup(); }
}
