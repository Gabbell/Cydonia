#include <Window/GLFWWindow.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <GLFW/glfw3.h>

namespace cyd
{
bool Window::init( uint32_t width, uint32_t height, const std::string& title )
{
   if( !glfwInit() )
   {
      CYDASSERT_AND_RETURN( !"GLFW: Init failed", false );
   }

   if( !glfwVulkanSupported() )
   {
      CYDASSERT_AND_RETURN( !"GLFW: Vulkan not supported", false );
   }

   m_extent = { width, height };

   // Creating GLFWwindow
   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );  // Tell GLFW we do not need a GL context
   m_glfwWindow = glfwCreateWindow( width, height, title.c_str(), nullptr, nullptr );
   CYDASSERT_AND_RETURN( m_glfwWindow && "Could not create GLFW window", false );

   // Populating extensions
   uint32_t extensionsCount = 0;
   const char** extensions  = glfwGetRequiredInstanceExtensions( &extensionsCount );
   m_extensions             = std::vector<const char*>( extensions, extensions + extensionsCount );

#if _DEBUG
   m_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

   printf( "GLFW: Compiled with GLFW version %s\n", glfwGetVersionString() );

   return true;
}

bool Window::isRunning() const { return !glfwWindowShouldClose( m_glfwWindow ); }

Window::~Window()
{
   glfwDestroyWindow( m_glfwWindow );
   glfwTerminate();
}
}
