#pragma once

#include <ECS/Components/ComponentTypes.h>

namespace cyd
{
class RenderableComponent final : public BaseComponent
{
  public:
   RenderableComponent() = default;
   NON_COPIABLE( RenderableComponent );
   virtual ~RenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   bool init( const void* pDescription ) override;
};
}