#pragma once

#include <ECS/Components/BaseComponent.h>

#include <ECS/Components/ComponentTypes.h>

#include <string_view>
#include <vector>

// ================================================================================================
// Definition
// ================================================================================================
/*
 */
namespace CYD
{
struct Vertex;

class MeshComponent : public BaseComponent
{
  public:
   MeshComponent() = default;
   explicit MeshComponent( std::string_view assetName ) : asset( assetName ) {}
   COPIABLE( MeshComponent );
   virtual ~MeshComponent() = default;

   static constexpr ComponentType TYPE = ComponentType::MESH;

   // Path of the mesh asset
   std::string_view asset;
};
}
