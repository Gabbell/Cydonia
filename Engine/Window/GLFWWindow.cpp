#include <Window/GLFWWindow.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>

#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

namespace CYD
{
bool Window::init( uint32_t width, uint32_t height, const char* title )
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
   m_glfwWindow = glfwCreateWindow( width, height, title, nullptr, nullptr );
   CYDASSERT_AND_RETURN( m_glfwWindow && "Could not create GLFW window", false );

   // Assigning icon
   GLFWimage images[1];
   images[0].pixels =
       stbi_load( "CydoniaIcon.png", &images[0].width, &images[0].height, 0, 4 );  // rgba channels
   glfwSetWindowIcon( m_glfwWindow, 1, images );
   stbi_image_free( images[0].pixels );

   // Populating extensions
   uint32_t extensionsCount = 0;
   const char** extensions  = glfwGetRequiredInstanceExtensions( &extensionsCount );
   m_extensions             = std::vector<const char*>( extensions, extensions + extensionsCount );

#if CYD_DEBUG
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
