#include <Graphics/Vulkan/Surface.h>

#include <Common/Assert.h>

#include <Window/GLFWWindow.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/Instance.h>

#include <GLFW/glfw3.h>

namespace vk
{
Surface::Surface( const CYD::Window& window, const Instance& instance )
    : m_window( window ), m_instance( instance )
{
   VkResult result = glfwCreateWindowSurface(
       m_instance.getVKInstance(), m_window.getGLFWwindow(), nullptr, &m_vkSurface );
   CYD_ASSERT( result == VK_SUCCESS && "Surface: Could not create surface" );
}

Surface::~Surface()
{
   vkDestroySurfaceKHR( m_instance.getVKInstance(), m_vkSurface, nullptr );
   m_vkSurface = nullptr;
}
}