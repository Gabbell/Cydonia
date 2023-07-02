#include <Graphics/Scene/MaterialCache.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/GraphicsIO.h>

#include <json/json.hpp>

#include <fstream>

namespace CYD
{
// Asset Paths Constants
// ================================================================================================
static constexpr uint32_t MAX_STATIC_MATERIALS = 128;
static constexpr char STATIC_MATERIALS_PATH[]  = "Materials.json";
//static constexpr char MATERIALS_PATH[]         = "../Engine/Data/Materials/";

MaterialCache::MaterialCache() { initializeStaticMaterials(); }

MaterialIndex MaterialCache::addMaterial( const std::string& name, const Material::Description& desc )
{
   MaterialIndex index   = m_materials.insertObject( name, desc );
   m_materialNames[name] = index;

   return index;
}

void MaterialCache::removeMaterial( MaterialIndex /*index*/ )
{
   // TODO
}

void MaterialCache::load( CmdListHandle transferList, MaterialIndex index )
{
   m_materials[index]->load( transferList );
}

void MaterialCache::unload( MaterialIndex index ) { m_materials[index]->unload(); }

void MaterialCache::bind( CmdListHandle cmdList, MaterialIndex index, uint8_t set ) const
{
   m_materials[index]->bind( cmdList, set );
}

void MaterialCache::updateMaterial( MaterialIndex index, TextureHandle texture, Material::TextureSlot slot)
{
   return m_materials[index]->updateTexture( texture, slot );
}

MaterialIndex MaterialCache::getMaterialByName( const std::string& name ) const
{
   const auto& findIt = m_materialNames.find( name );
   if( findIt != m_materialNames.end() )
   {
      return findIt->second;
   }

   return INVALID_MATERIAL_IDX;
}

static PixelFormat StringToPixelFormat( const std::string& formatString )
{
   if( formatString == "RGBA32F" )
   {
      return PixelFormat::RGBA32F;
   }
   if( formatString == "RGB32F" )
   {
      return PixelFormat::RGB32F;
   }
   if( formatString == "R32F" )
   {
      return PixelFormat::R32F;
   }
   if( formatString == "RGBA8_SRGB" )
   {
      return PixelFormat::RGBA8_SRGB;
   }
   if( formatString == "RGBA8_UNORM" )
   {
      return PixelFormat::RGBA8_UNORM;
   }

   CYD_ASSERT( !"Materials: Could not recognize string as a pixel format" );
   return PixelFormat::RGBA32F;
}

static ImageType StringToTextureType( const std::string& formatString )
{
   if( formatString == "TEXTURE1D" )
   {
      return ImageType::TEXTURE_1D;
   }
   if( formatString == "TEXTURE2D" )
   {
      return ImageType::TEXTURE_2D;
   }
   if( formatString == "TEXTURE3D" )
   {
      return ImageType::TEXTURE_3D;
   }

   CYD_ASSERT( !"Materials: Could not recognize string as a texture type" );
   return ImageType::TEXTURE_2D;
}

void MaterialCache::initializeStaticMaterials()
{
   // Parse material infos from JSON description
   nlohmann::json materialDescriptions;
   std::ifstream materialsFile( STATIC_MATERIALS_PATH );

   if( !materialsFile.is_open() )
   {
      CYD_ASSERT( !"StaticMaterials: Could not find materials file" );
      return;
   }

   materialsFile >> materialDescriptions;

   const auto& materials = materialDescriptions.front();

   printf( "======== Initializing Static Materials ========\n" );

   for( uint32_t materialIdx = 0; materialIdx < materials.size(); ++materialIdx )
   {
      const auto& material           = materials[materialIdx];
      const std::string materialName = material["NAME"];

      // Making sure there is no name duplication
#if CYD_ASSERTIONS_ENABLED
      const auto& dupIt = m_materialNames.find( materialName );
      if( dupIt != m_materialNames.end() )
      {
         CYD_ASSERT( !"Materials: Name already taken, ignoring" );
         continue;
      }
#endif

      Material::Description desc;

      const auto& texturesIt = material.find( "TEXTURES" );
      if( texturesIt != material.end() )
      {
         TextureDescription textureDesc;

         for( const auto& texture : *texturesIt )
         {
            textureDesc.name   = texture["NAME"];
            textureDesc.format = StringToPixelFormat( texture["FORMAT"] );
            textureDesc.type   = StringToTextureType( texture["TYPE"] );
            textureDesc.stages = PipelineStage::FRAGMENT_STAGE;
            textureDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

            std::string path   = "";
            const auto& pathIt = texture.find( "PATH" );
            if( pathIt != texture.end() )
            {
               path = texture["PATH"];
            }

            desc.addTexture( textureDesc, path );
         }
      }

      MaterialIndex index           = m_materials.insertObject();
      m_materialNames[materialName] = index;

      printf( "Added material --> %s\n", materialName.c_str() );
   }

   printf( "======== End Static Materials ========\n" );

   return;
}
}