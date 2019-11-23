#include <Core/Window/GLFWWindow.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <GLFW/glfw3.h>

cyd::Window::Window( uint32_t width, uint32_t height, const std::string& title )
{
   if( !glfwInit() )
   {
      CYDASSERT( !"GLFW: Init failed" );
      return;
   }

   if( !glfwVulkanSupported() )
   {
      CYDASSERT( !"GLFW: Vulkan not supported" );
      return;
   }

   _extent = { width, height };

   // Creating GLFWwindow
   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );  // Tell GLFW we do not need a GL context
   _glfwWindow = glfwCreateWindow( width, height, title.c_str(), nullptr, nullptr );
   CYDASSERT( _glfwWindow && "Could not create GLFW window" );

   // Populating extensions
   uint32_t extensionsCount = 0;
   const char** extensions  = glfwGetRequiredInstanceExtensions( &extensionsCount );
   _extensions              = std::vector<const char*>( extensions, extensions + extensionsCount );

#if _DEBUG
   _extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

   printf( "GLFW: Compiled with GLFW version %s\n", glfwGetVersionString() );
}

bool cyd::Window::isRunning() const { return !glfwWindowShouldClose( _glfwWindow ); }

cyd::Window::~Window()
{
   glfwDestroyWindow( _glfwWindow );
   glfwTerminate();
}
