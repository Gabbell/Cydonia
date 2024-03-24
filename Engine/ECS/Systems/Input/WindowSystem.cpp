#include <ECS/Systems/Input/WindowSystem.h>

#include <Input/GLFWWindow.h>

#include <Graphics/GRIS/RenderInterface.h>

#include <ECS/EntityManager.h>
#include <ECS/SharedComponents/InputComponent.h>
#include <ECS/SharedComponents/SceneComponent.h>

#include <Profiling.h>

#include <GLFW/glfw3.h>

namespace CYD
{
WindowSystem::WindowSystem( Window& window ) : m_window( window )
{
   // Settings instance of input interpreter to this window
   // TODO Maybe there's a better way to do this?
   glfwSetWindowUserPointer( m_window.getGLFWwindow(), this );

   // Callback wrappers
   auto mainKeyCallback = []( GLFWwindow* window, int key, int scancode, int action, int mods ) {
      static_cast<WindowSystem*>( glfwGetWindowUserPointer( window ) )
          ->_keyCallback( window, key, scancode, action, mods );
   };

   auto mainMouseCallback = []( GLFWwindow* window, int button, int action, int mods ) {
      static_cast<WindowSystem*>( glfwGetWindowUserPointer( window ) )
          ->_mouseCallback( window, button, action, mods );
   };

   auto mainResizeCallback = []( GLFWwindow* window, int width, int height ) {
      static_cast<WindowSystem*>( glfwGetWindowUserPointer( window ) )
          ->_resizeCallback( window, width, height );
   };

   // Registering callbacks
   glfwSetKeyCallback( m_window.getGLFWwindow(), mainKeyCallback );
   glfwSetMouseButtonCallback( m_window.getGLFWwindow(), mainMouseCallback );
   glfwSetFramebufferSizeCallback( m_window.getGLFWwindow(), mainResizeCallback );
}

void WindowSystem::tick( double /*deltaS*/ )
{
   CYD_TRACE();

   InputComponent& input = m_ecs->getSharedComponent<InputComponent>();
   SceneComponent& scene = m_ecs->getSharedComponent<SceneComponent>();

   // Update cursor information
   double curPosX, curPosY;
   glfwGetCursorPos( m_window.getGLFWwindow(), &curPosX, &curPosY );
   const glm::vec2 newCursorPos( curPosX, curPosY );
   input.cursorDelta  = newCursorPos - input.curCursorPos;
   input.curCursorPos = newCursorPos;

   // Updating scene extent to match window extent if needed
   const Extent2D& windowExtent = m_window.getExtent();
   if( scene.extent != m_window.getExtent() )
   {
      // Keeping the main color and depth textures in sync with the size of the window and swapchain
      const uint32_t newWidth  = windowExtent.width;
      const uint32_t newHeight = windowExtent.height;

      // Update scene
      scene.viewport = {
          0.0f, 0.0f, static_cast<float>( newWidth ), static_cast<float>( newHeight ) };
      scene.scissor = {
          { 0, 0 }, { static_cast<uint32_t>( newWidth ), static_cast<uint32_t>( newHeight ) } };

      scene.mainFramebuffer.detach( Framebuffer::COLOR );
      scene.mainFramebuffer.detach( Framebuffer::DEPTH );
      GRIS::DestroyTexture( scene.mainColor );
      GRIS::DestroyTexture( scene.mainDepth );

      // Main Color
      TextureDescription colorDesc;
      colorDesc.format = PixelFormat::RGBA32F;
      colorDesc.width  = newWidth;
      colorDesc.height = newHeight;
      colorDesc.usage =
          ImageUsage::COLOR | ImageUsage::SAMPLED | ImageUsage::STORAGE | ImageUsage::TRANSFER_SRC;
      colorDesc.stages = PipelineStage::FRAGMENT_STAGE | PipelineStage::COMPUTE_STAGE;
      colorDesc.type   = ImageType::TEXTURE_2D;
      colorDesc.name   = "Main Color";

      scene.mainColor = GRIS::CreateTexture( colorDesc );

      // Main Depth
      TextureDescription depthDesc;
      depthDesc.format = PixelFormat::D32_SFLOAT;
      depthDesc.width  = newWidth;
      depthDesc.height = newHeight;
      depthDesc.usage  = ImageUsage::DEPTH_STENCIL | ImageUsage::SAMPLED;
      depthDesc.stages = PipelineStage::FRAGMENT_STAGE | PipelineStage::COMPUTE_STAGE;
      depthDesc.type   = ImageType::TEXTURE_2D;
      depthDesc.name   = "Main Depth";

      scene.mainDepth = GRIS::CreateTexture( depthDesc );

      scene.mainFramebuffer.resize( newWidth, newHeight );
      scene.mainFramebuffer.attach(
          Framebuffer::COLOR, scene.mainColor, Access::COLOR_ATTACHMENT_WRITE );
      scene.mainFramebuffer.attach(
          Framebuffer::DEPTH, scene.mainDepth, Access::DEPTH_STENCIL_ATTACHMENT_WRITE );

      scene.resolutionChanged = true;
      scene.extent            = windowExtent;
   }
   else
   {
      scene.resolutionChanged = false;
   }
}

void WindowSystem::_keyCallback(
    GLFWwindow* window,
    int key,
    int /*scancode*/,
    int action,
    int mods )
{
   if( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
   {
      glfwSetWindowShouldClose( window, true );
   }

   InputComponent& input = m_ecs->getSharedComponent<InputComponent>();

   if( key == GLFW_KEY_W )
   {
      if( action == GLFW_PRESS )
      {
         input.goingForwards = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingForwards = false;
      }
   }

   if( key == GLFW_KEY_S )
   {
      if( action == GLFW_PRESS )
      {
         input.goingBackwards = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingBackwards = false;
      }
   }

   if( key == GLFW_KEY_A )
   {
      if( action == GLFW_PRESS )
      {
         input.goingLeft = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingLeft = false;
      }
   }

   if( key == GLFW_KEY_D )
   {
      if( action == GLFW_PRESS )
      {
         input.goingRight = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingRight = false;
      }
   }

   if( key == GLFW_KEY_SPACE )
   {
      if( action == GLFW_PRESS )
      {
         input.goingUp = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingUp = false;
      }
   }

   if( key == GLFW_KEY_LEFT_SHIFT )
   {
      if( action == GLFW_PRESS )
      {
         input.goingDown = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.goingDown = false;
      }
   }

   if( key == GLFW_KEY_LEFT_CONTROL )
   {
      if( action == GLFW_PRESS )
      {
         input.sprinting = true;
      }
      else if( action == GLFW_RELEASE )
      {
         input.sprinting = false;
      }
   }

   if( action == GLFW_PRESS && mods == GLFW_MOD_CONTROL && key == GLFW_KEY_R )
   {
      GRIS::ReloadShaders();
   }
}

void WindowSystem::_mouseCallback( GLFWwindow* window, int button, int action, int /*mods*/ )
{
   InputComponent& input = m_ecs->getSharedComponent<InputComponent>();

   if( button == GLFW_MOUSE_BUTTON_RIGHT )
   {
      if( action == GLFW_PRESS )
      {
         glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
         input.rightClick = true;
      }
      else if( action == GLFW_RELEASE )
      {
         glfwSetInputMode( window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
         input.rightClick = false;
      }
   }
}

void WindowSystem::_resizeCallback( GLFWwindow* window, int width, int height )
{
   m_window.setExtent( width, height );
}
}
