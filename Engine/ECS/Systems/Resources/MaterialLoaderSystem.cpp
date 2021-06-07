#include <ECS/Systems/Resources/MaterialLoaderSystem.h>

#include <Graphics/AssetStash.h>
#include <Graphics/RenderGraph.h>
#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
void MaterialLoaderSystem::tick( double /*deltaS*/ )
{
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER, "MaterialLoaderSystem" );

   GRIS::StartRecordingCommandList( transferList );

   for( const auto& entityEntry : m_entities )
   {
      MaterialComponent& material = *std::get<MaterialComponent*>( entityEntry.arch );

      if( material.data.albedo || material.data.normals || material.data.metalness ||
          material.data.roughness || material.data.ao )
      {
         continue;
      }

      m_assets.loadMaterialFromPath( transferList, material.asset );

      const Material& loadedMaterial = m_assets.getMaterial( material.asset );
      material.data.albedo           = loadedMaterial.albedo;
      material.data.normals          = loadedMaterial.normals;
      material.data.metalness        = loadedMaterial.metalness;
      material.data.roughness        = loadedMaterial.roughness;
      material.data.ao               = loadedMaterial.ao;
   }

   // We don't want to spend more time on loading these entities' resources
   m_entities.clear();

   GRIS::EndRecordingCommandList( transferList );

   RenderGraph::AddPass( transferList );
}
}
