#include <ECS/Systems/Resources/MaterialLoaderSystem.h>

#include <Graphics/AssetStash.h>
#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
void MaterialLoaderSystem::tick( double /*deltaS*/ )
{
   CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   for( const auto& entityEntry : m_entities )
   {
      MaterialComponent& material = *std::get<MaterialComponent*>( entityEntry.arch );

      if( material.albedo || material.normals || material.metalness || material.roughness ||
          material.ao || material.height )
      {
         continue;
      }

      const Material& loadedMaterial = m_assets.loadMaterial( transferList, material.asset );

      material.albedo    = loadedMaterial.albedo;
      material.normals   = loadedMaterial.normals;
      material.metalness = loadedMaterial.metalness;
      material.roughness = loadedMaterial.roughness;
      material.ao        = loadedMaterial.ao;
      material.height    = loadedMaterial.height;
   }

   // We don't want to spend more time on loading these entities' resources
   m_entities.clear();

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}
}
