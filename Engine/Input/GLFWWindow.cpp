#include <Input/GLFWWindow.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>

#include <Profiling.h>

#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

namespace CYD
{
bool Window::init( uint32_t width, uint32_t height, const char* title )
{
   if( !glfwInit() )
   {
      CYD_ASSERT_AND_RETURN( !"GLFW: Init failed", return false; );
   }

   if( !glfwVulkanSupported() )
   {
      CYD_ASSERT_AND_RETURN( !"GLFW: Vulkan not supported", return false; );
   }

   m_extent = { width, height };

   // Creating GLFWwindow
   glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );  // Tell GLFW we do not need a GL context
   m_glfwWindow = glfwCreateWindow( width, height, title, nullptr, nullptr );
   CYD_ASSERT_AND_RETURN( m_glfwWindow && "Could not create GLFW window", return false; );

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

#if CYD_DEBUG || CYD_PROFILING
   m_extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

   printf( "GLFW: Compiled with GLFW version %s\n", glfwGetVersionString() );

   return true;
}

bool Window::isRunning() const { return !glfwWindowShouldClose( m_glfwWindow ); }

void Window::poll() const
{
   CYD_TRACE();
   glfwPollEvents();
}

Window::~Window()
{
   glfwDestroyWindow( m_glfwWindow );
   glfwTerminate();
}
}
