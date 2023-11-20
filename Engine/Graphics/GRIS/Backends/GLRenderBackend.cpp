#include <Graphics/GRIS/Backends/GLRenderBackend.h>

#include <Graphics/GraphicsTypes.h>

#include <Input/GLFWWindow.h>

namespace CYD
{
// =================================================================================================
// Implementation
class GLRenderBackendImp
{
  public:
   GLRenderBackendImp( const Window& ) { printf( "Initializing GL\n" ); }
   ~GLRenderBackendImp() {}
};
}
