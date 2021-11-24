// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include <Window/SDLWindow.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

static void handleSDLError()
{
   const char* error = SDL_GetError();
   CYDASSERT( !error );
}

bool CYD::Window::init( uint32_t width, uint32_t height, const std::string& title )
{
   m_extent = {width, height};

   if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
   {
      handleSDLError();
      return false;
   }

   // Creating SDLm_Window
   m_sdlWindow = SDL_CreateWindow(
       title.c_str(),
       SDL_WINDOWPOS_CENTERED,
       SDL_WINDOWPOS_CENTERED,
       _extent.width,
       _extent.height,
       SDL_WINDOW_VULKAN );
   CYDASSERT_AND_RETURN( m_sdlWindow && "Could not create SDL window", false );

   // Populating extensions
   uint32_t extensionsCount = 0;
   if( !SDL_Vulkan_GetInstanceExtensions( _sdlWindow, &extensionsCount, nullptr ) )
   {
      handleSDLError();
      return false;
   }

   _extensions.resize( extensionsCount );

   if( !SDL_Vulkan_GetInstanceExtensions( _sdlWindow, &extensionsCount, _extensions.data() ) )
   {
      handleSDLError();
      return false;
   }

#if m_DEBUG
   _extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

   SDLm_version linkedVersion;
   SDL_GetVersion( &linkedVersion );
   printf(
       "SDL Window: Linked with SDL version %d.%d.%d\n",
       linkedVersion.major,
       linkedVersion.minor,
       linkedVersion.patch );
}

bool CYD::Window::uninit()
{
   SDL_DestroyWindow( m_sdlWindow );
   SDL_Quit();
}

CYD::Window::~Window() { uninit(); }
