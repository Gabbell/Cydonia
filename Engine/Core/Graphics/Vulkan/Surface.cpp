#include <Core/Graphics/Vulkan/Surface.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Window/GLFWWindow.h>

#include <Core/Graphics/Vulkan/Instance.h>

#include <GLFW/glfw3.h>

cyd::Surface::Surface( const Instance& instance, const Window& window )
    : _window( window ), _instance( instance )
{
   VkResult result = glfwCreateWindowSurface(
       _instance.getVKInstance(), _window.getGLFWwindow(), nullptr, &_vkSurface );
   CYDASSERT( result == VK_SUCCESS && "Surface: Could not create surface" );
}

cyd::Surface::~Surface()
{
   vkDestroySurfaceKHR( _instance.getVKInstance(), _vkSurface, nullptr );
   _vkSurface = nullptr;
}
