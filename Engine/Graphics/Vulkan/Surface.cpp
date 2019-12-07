#include <Graphics/Vulkan/Surface.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Window/GLFWWindow.h>

#include <Graphics/Vulkan/Instance.h>

#include <GLFW/glfw3.h>

namespace vk
{
Surface::Surface( const cyd::Window& window, const Instance& instance )
    : _window( window ), _instance( instance )
{
   VkResult result = glfwCreateWindowSurface(
       _instance.getVKInstance(), _window.getGLFWwindow(), nullptr, &_vkSurface );
   CYDASSERT( result == VK_SUCCESS && "Surface: Could not create surface" );
}

Surface::~Surface()
{
   vkDestroySurfaceKHR( _instance.getVKInstance(), _vkSurface, nullptr );
   _vkSurface = nullptr;
}
}