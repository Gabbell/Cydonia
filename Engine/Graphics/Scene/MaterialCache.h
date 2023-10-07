#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>
#include <Graphics/Scene/Material.h>

#include <Common/ObjectPool.h>

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace CYD
{
class MaterialCache final
{
  public:
   MaterialCache();
   NON_COPIABLE( MaterialCache );
   virtual ~MaterialCache() = default;

   // void cleanup();

   MaterialIndex getMaterialByName( const std::string& name ) const;

   MaterialIndex addMaterial( const std::string& name, const Material::ResourcesDescription& desc );
   void removeMaterial( MaterialIndex index );

   // Loads, unloads and binds GPU resources
   void load( CmdListHandle transferList, MaterialIndex index );
   void unload( MaterialIndex index );
   void bind( CmdListHandle cmdList, MaterialIndex index, uint8_t set ) const;

   void updateMaterial( MaterialIndex, TextureHandle texture, Material::TextureSlot slot );

  private:
   void initializeStaticMaterials();

   EMP::ObjectPool<Material> m_materials;
   std::unordered_map<std::string, MaterialIndex> m_materialNames;
};
}
