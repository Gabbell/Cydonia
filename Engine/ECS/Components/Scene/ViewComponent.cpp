#include <ECS/Components/Scene/ViewComponent.h>

namespace CYD
{
ViewComponent::ViewComponent( EntityHandle fitToEntity, bool reverseZ )
    : fitToEntity( fitToEntity ), reverseZ( reverseZ )
{
   projMode = ProjectionMode::ORTHOGRAPHIC;
}

ViewComponent::ViewComponent( float fov, float near, float far, bool reverseZ )
    : near( near ), far( far ), reverseZ( reverseZ )
{
   projMode               = ProjectionMode::PERSPECTIVE;
   params.perspective.fov = fov;
}

ViewComponent::ViewComponent(
    float left,
    float right,
    float bottom,
    float top,
    float near,
    float far,
    bool reverseZ )
    : near( near ), far( far ), reverseZ( reverseZ )
{
   projMode            = ProjectionMode::ORTHOGRAPHIC;
   params.ortho.left   = left;
   params.ortho.right  = right;
   params.ortho.bottom = bottom;
   params.ortho.top    = top;
}
}
