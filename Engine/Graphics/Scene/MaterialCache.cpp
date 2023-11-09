#include <Graphics/Scene/MaterialCache.h>
#include <Graphics/Scene/MaterialCache.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/GraphicsIO.h>

#include <Multithreading/ThreadPool.h>

#include <Profiling.h>

#include <json/json.hpp>

#include <fstream>

namespace CYD
{
// Asset Paths Constants
// ================================================================================================
static constexpr uint32_t MAX_STATIC_MATERIALS = 128;
static constexpr char STATIC_MATERIALS_PATH[]  = "Materials.json";
static const char MATERIAL_PATH[]              = "../../Engine/Data/Materials/";

MaterialCache::MaterialCache( EMP::ThreadPool& threadPool ) : m_threadPool( threadPool )
{
   _initializeStaticMaterials();
}

MaterialCache::Material::~Material()
{
   for( uint32_t i = 0; i < TextureSlot::COUNT; ++i )
   {
      TextureEntry& textureEntry = textures[i];
      if( textureEntry.texHandle )
      {
         GRIS::DestroyTexture( textureEntry.texHandle );
      }
   }
}

void MaterialCache::loadToRAM( Material& material )
{
   CYD_TRACE();

   CYD_ASSERT_AND_RETURN( material.currentState == MaterialCache::State::UNINITIALIZED, return; );

   material.currentState = State::LOADING_TO_RAM;

   for( uint32_t i = 0; i < TextureSlot::COUNT; ++i )
   {
      TextureEntry& textureEntry = material.textures[i];

      // If we have a path, load from storage. If not, this texture is probably managed elsewhere
      if( !textureEntry.path.empty() )
      {
         int width;
         int height;
         int size;

         textureEntry.imageData = GraphicsIO::LoadImage(
             textureEntry.path, textureEntry.desc.format, width, height, size );

         textureEntry.desc.width  = width;
         textureEntry.desc.height = height;
      }
   }

   material.currentState = State::LOADED_TO_RAM;
}

void MaterialCache::loadToVRAM( CmdListHandle cmdList, Material& material )
{
   CYD_TRACE();

   CYD_ASSERT_AND_RETURN( material.currentState == MaterialCache::State::LOADED_TO_RAM, return; );

   material.currentState = State::LOADING_TO_VRAM;

   for( uint32_t i = 0; i < TextureSlot::COUNT; ++i )
   {
      TextureEntry& textureEntry = material.textures[i];
      if( textureEntry.imageData )
      {
         textureEntry.texHandle =
             GRIS::CreateTexture( cmdList, textureEntry.desc, textureEntry.imageData );
      }
   }

   // This only works because we guarantee that the LOAD command list is first in the
   // render graph so the transfer command list will have  uploaded this mesh data
   // when the time comes to render
   material.currentState = State::LOADED_TO_VRAM;
}

MaterialCache::State MaterialCache::progressLoad( CmdListHandle cmdList, MaterialIndex materialIdx )
{
   Material* pMaterial = m_materials[materialIdx];
   CYD_ASSERT( pMaterial );

   Material& material = *pMaterial;

   if( material.currentState == State::UNINITIALIZED )
   {
      // This material is not loaded, start a load job
      m_threadPool.submit( MaterialCache::loadToRAM, material );
   }

   if( material.currentState == State::LOADED_TO_RAM )
   {
      loadToVRAM( cmdList, material );
   }

   // We are setting the LOADED_TO_VRAM state right after loadToVRAM is called
   // This only works because we guarantee that the LOAD command list is first in the
   // render graph so the transfer command list will have  uploaded this mesh data
   // when the time comes to render. We can also free the data here because it has been uploaded
   // to a staging buffer already
   // TODO Make it so that we can right away upload to a staging buffer instead of having to copy
   if( material.currentState == State::LOADED_TO_VRAM )
   {
      // Make sure we're freeing RAM
      for( uint32_t i = 0; i < TextureSlot::COUNT; ++i )
      {
         TextureEntry& textureEntry = material.textures[i];
         if( textureEntry.imageData )
         {
            GraphicsIO::FreeImage( textureEntry.imageData );
            textureEntry.imageData = nullptr;
         }
      }
   }

   return material.currentState;
}

void MaterialCache::bind( CmdListHandle cmdList, MaterialIndex index, uint8_t set ) const
{
   const Material* material = m_materials[index];
   CYD_ASSERT( material );

   for( uint32_t i = 0; i < TextureSlot::COUNT; ++i )
   {
      const TextureEntry& textureEntry = material->textures[i];
      if( textureEntry.texHandle )
      {
         // TODO Allow non-contiguous bindings?
         // TODO Allow different sampling?
         GRIS::BindTexture( cmdList, textureEntry.texHandle, i, set );
      }
   }
}

void MaterialCache::updateMaterial( MaterialIndex index, TextureSlot slot, TextureHandle texHandle )
{
   Material* material = m_materials[index];
   CYD_ASSERT( material );

   TextureEntry& textureEntry = material->textures[slot];
   if( !textureEntry.texHandle )
   {
      textureEntry.texHandle = texHandle;
   }
}

MaterialIndex MaterialCache::getMaterialIndexByName( std::string_view name ) const
{
   const auto& findIt = m_materialNames.find( std::string( name ) );
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
   if( formatString == "RGBA16F" )
   {
      return PixelFormat::RGBA16F;
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
   if( formatString == "R16_UNORM" )
   {
      return PixelFormat::R16_UNORM;
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

void MaterialCache::_initializeStaticMaterials()
{
   // Parse material infos from JSON description
   nlohmann::json materialDescriptions;
   std::ifstream materialsFile( STATIC_MATERIALS_PATH );

   if( !materialsFile.is_open() )
   {
      CYD_ASSERT( !"StaticMaterials: Could not find static materials file" );
      return;
   }

   materialsFile >> materialDescriptions;

   const auto& materials = materialDescriptions.front();

   printf( "======== Initializing Static Materials ========\n" );

   for( uint32_t materialIdx = 0; materialIdx < materials.size(); ++materialIdx )
   {
      const auto& materialJson       = materials[materialIdx];
      const std::string materialName = materialJson["NAME"];

      // Making sure there is no name duplication
#if CYD_ASSERTIONS_ENABLED
      const auto& dupIt = m_materialNames.find( materialName );
      if( dupIt != m_materialNames.end() )
      {
         CYD_ASSERT( !"Materials: Name already taken, ignoring" );
         continue;
      }
#endif

      MaterialIndex index = m_materials.insertObject();
      Material* material  = m_materials[index];
      CYD_ASSERT( material );

      const auto& texturesIt = materialJson.find( "TEXTURES" );
      if( texturesIt != materialJson.end() )
      {
         uint32_t index = 0;
         for( const auto& texture : *texturesIt )
         {
            std::string path   = "";
            const auto& pathIt = texture.find( "PATH" );
            if( pathIt != texture.end() )
            {
               path = std::string( MATERIAL_PATH );
               path += texture["PATH"];
            }

            TextureEntry& textureEntry = material->textures[index];
            textureEntry.path          = path;
            textureEntry.desc.format   = StringToPixelFormat( texture["FORMAT"] );
            textureEntry.desc.type     = StringToTextureType( texture["TYPE"] );
            textureEntry.desc.stages   = PipelineStage::FRAGMENT_STAGE;
            textureEntry.desc.usage    = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

            index++;
         }
      }

      m_materialNames[materialName] = index;

      printf( "Added material --> %s\n", materialName.c_str() );
   }

   printf( "======== End Static Materials ========\n" );

   return;
}
}