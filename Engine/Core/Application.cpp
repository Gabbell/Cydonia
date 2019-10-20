#include <Core/Application.h>

#include <Core/Common/Assert.h>
#include <Core/Graphics/Types.h>

#include <Core/Window/Window.h>

#include <Core/Graphics/Instance.h>
#include <Core/Graphics/Surface.h>
#include <Core/Graphics/Device.h>
#include <Core/Graphics/DeviceHerder.h>
#include <Core/Graphics/CommandBuffer.h>
#include <Core/Graphics/Swapchain.h>

#include <SDL2/SDL.h>

#include <chrono>

static cyd::PipelineInfo pipInfo;

static cyd::Swapchain* swapchain = nullptr;
static cyd::Device* device       = nullptr;

cyd::Application::Application( uint32_t width, uint32_t height, const std::string& title )
    : _running( true )
{
   _window   = std::make_unique<Window>( width, height, title );
   _instance = std::make_unique<Instance>( *_window );
   _surface  = std::make_unique<Surface>( *_instance, *_window );
   _dh       = std::make_unique<DeviceHerder>( *_instance, *_window, *_surface );

   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.usage      = AttachmentUsage::PRESENTATION;

   RenderPassInfo renderPassInfo = {};
   renderPassInfo.attachments.push_back( colorPresentation );

   SwapchainInfo scInfo = {};
   scInfo.extent        = _window->getExtent();
   scInfo.format        = PixelFormat::BGRA8_UNORM;
   scInfo.space         = ColorSpace::SRGB_NONLINEAR;
   scInfo.mode          = PresentMode::MAILBOX;

   PipelineLayoutInfo pipLayout = {};
   pipInfo.renderPass           = renderPassInfo;
   pipInfo.drawPrim             = DrawPrimitive::TRIANGLES;
   pipInfo.extent               = _window->getExtent();
   pipInfo.polyMode             = PolygonMode::FILL;
   pipInfo.shaders              = { "default_vert.spv", "default_frag.spv" };
   pipInfo.pipLayout            = pipLayout;

   device    = _dh->getMainDevice();
   swapchain = device->createSwapchain( scInfo );
}

void cyd::Application::startLoop()
{
   static auto start = std::chrono::high_resolution_clock::now();

   preLoop();
   while( _running )  // Main loop
   {
      // Calculate delta time
      const std::chrono::duration<double> deltaTime =
          std::chrono::high_resolution_clock::now() - start;

      // Reset clock
      start = std::chrono::high_resolution_clock::now();

      tick( deltaTime.count() );
      drawFrame( deltaTime.count() );
   }
   postLoop();
}

void cyd::Application::preLoop() {}
void cyd::Application::tick( double deltaTime )
{
   // TODO Make an input manager
   SDL_Event event;
   while( SDL_PollEvent( &event ) )
   {
      switch( event.type )
      {
         case SDL_KEYDOWN:
            if( event.key.keysym.sym == SDLK_ESCAPE )
            {
               _running = false;
            }
            break;

         default:
            break;
      }
   }
}
void cyd::Application::drawFrame( double deltaTime )
{
   // Drawing in the swapchain
   std::shared_ptr<CommandBuffer> cmds = device->createCommandBuffer( Usage::GRAPHICS );

   Extent extent = _window->getExtent();

   cmds->startRecording();
   cmds->setPipeline( pipInfo );
   cmds->setViewport( extent.width, extent.height );
   cmds->beginPass( swapchain );
   cmds->draw();
   cmds->endPass();
   cmds->endRecording();

   swapchain->present();

   device->cleanup();
}
void cyd::Application::postLoop() {}

cyd::Application::~Application() {}
