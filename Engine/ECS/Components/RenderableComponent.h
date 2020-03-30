#pragma once

#include <ECS/Components/BaseComponent.h>

#include <Handles/Handle.h>

#include <ECS/Components/ComponentTypes.h>
#include <ECS/Components/RenderableTypes.h>

#include <vector>
#include <string>

/*
This class is the base class for different types of renderables. You must inherit from this class
for the render system to be able to see the entity. This component is also abstract so you cannot
create entities with it. You need to use a renderable component with a pipeline attached (Phong,
PBR, etc.)
*/
namespace cyd
{
class RenderableComponent : public BaseComponent
{
  public:
   COPIABLE( RenderableComponent )
   virtual ~RenderableComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::RENDERABLE;

   RenderableType type = RenderableType::UNKNOWN;

   // TODO Duplicate renderable data like mesh or textures should point to the same handle
   VertexBufferHandle vertexBuffer;
   IndexBufferHandle indexBuffer;
   uint32_t indexCount  = 0;
   uint32_t vertexCount = 0;

  protected:
   RenderableComponent() = default;
   RenderableComponent( RenderableType aType ) : type( aType ) {}
};
}