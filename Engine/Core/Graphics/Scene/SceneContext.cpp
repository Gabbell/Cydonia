#include <Core/Graphics/Scene/SceneContext.h>

#include <Core/Graphics/Scene/Camera.h>

cyd::SceneContext::SceneContext( const Rectangle& viewport )
{
   _camera = std::make_unique<Camera>( viewport );
}

cyd::SceneContext::~SceneContext() {}
