#include <Applications/VKApplication.h>

#include <Graphics/RenderInterface.h>

#include <Handles/Handle.h>

namespace cyd
{
VKApplication::VKApplication( uint32_t width, uint32_t height, const std::string& title )
    : Application( width, height, title )
{
   initRenderBackend<API::VK>( *_window );
}

VKApplication::~VKApplication() { uninitRenderBackend(); }
}