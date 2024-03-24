#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/GraphicsTypes.h>

#include <string_view>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
class MeshComponent final : public BaseComponent
{
  public:
   MeshComponent() = default;
   explicit MeshComponent( const std::string_view name ) : name( name ) {}
   COPIABLE( MeshComponent );
   virtual ~MeshComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MESH;

   std::string_view name;

   MeshIndex meshIdx    = INVALID_MESH_IDX;
   uint32_t vertexCount = 0;
   uint32_t indexCount  = 0;
};
}
