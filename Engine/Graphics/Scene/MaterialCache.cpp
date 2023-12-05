#include <Graphics/Scene/MaterialCache.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/GRIS/TextureCache.h>
#include <Graphics/Utility/GraphicsIO.h>

#include <Multithreading/ThreadPool.h>

#include <Profiling.h>

#include <json/json.hpp>

#include <fstream>

namespace CYD
{
// Asset Paths Constants
// ================================================================================================
static constexpr char STATIC_MATERIALS_PATH[] = "Materials.json";
static const char MATERIAL_PATH[]             = "../../Engine/Data/Materials/";

MaterialCache::MaterialCache( EMP::ThreadPool& threadPool ) : m_threadPool( threadPool )
{
   _initializeStaticMaterials();
}

MaterialCache::Material::~Material()
{
   for( uint32_t i = 0; i < TextureSlot::COUNT; ++i )
   {
      // If we have a handle and we loaded from storage, we own the texture
      TextureEntry& textureEntry = textures[i];
      if( textureEntry.texHandle && needsLoadFromStorage )
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
         uint32_t size = 0;

         textureEntry.imageData = GraphicsIO::LoadImage(
             textureEntry.path,
             textureEntry.desc.format,
             textureEntry.desc.width,
             textureEntry.desc.height,
             size );
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

         if( textureEntry.desc.generateMipmaps )
         {
            // Create the mip maps
            GRIS::GenerateMipmaps( cmdList, textureEntry.texHandle );
         }
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
      if( material.needsLoadFromStorage )
      {
         // This material is not loaded, start a load job
         m_threadPool.submit( MaterialCache::loadToRAM, material );
      }
      else
      {
         material.currentState = State::LOADED_TO_VRAM;
      }
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

void MaterialCache::bindSlot(
    CmdListHandle cmdList,
    MaterialIndex index,
    TextureSlot slot,
    uint8_t set ) const
{
   const Material* material = m_materials[index];
   CYD_ASSERT( material );

   // TODO Allow non-contiguous bindings?
   // TODO Allow different sampling?
   SamplerInfo sampler;
   sampler.addressMode   = AddressMode::REPEAT;
   sampler.magFilter     = Filter::LINEAR;
   sampler.minFilter     = Filter::LINEAR;
   sampler.minLod        = 0.0f;
   sampler.maxLod        = 32.0f;
   sampler.maxAnisotropy = 16.0f;

   const TextureEntry& textureEntry = material->textures[slot];
   if( textureEntry.texHandle )
   {
      GRIS::BindTexture( cmdList, textureEntry.texHandle, sampler, slot, set );
   }
   else if( textureEntry.useFallback || !textureEntry.path.empty() )
   {
      switch( textureEntry.fallback )
      {
         case TextureFallback::BLACK:
            GRIS::TextureCache::BindBlackTexture( cmdList, slot, set );
            break;
         case TextureFallback::WHITE:
            GRIS::TextureCache::BindWhiteTexture( cmdList, slot, set );
            break;
         case TextureFallback::PINK:
            GRIS::TextureCache::BindPinkTexture( cmdList, slot, set );
            break;
      }
   }
}

void MaterialCache::bind( CmdListHandle cmdList, MaterialIndex index, uint8_t set ) const
{
   for( uint32_t i = 0; i < TextureSlot::COUNT; ++i )
   {
      bindSlot( cmdList, index, static_cast<TextureSlot>( i ), set );
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

MaterialIndex MaterialCache::findMaterial( std::string_view name ) const
{
   const auto& findIt = m_materialNames.find( std::string( name ) );
   if( findIt != m_materialNames.end() )
   {
      return findIt->second;
   }

   return INVALID_MATERIAL_IDX;
}

MaterialIndex MaterialCache::addMaterial( std::string_view name )
{
   const std::string nameString( name );

   CYD_ASSERT( m_materialNames.find( nameString ) == m_materialNames.end() );

   const size_t newIdx         = m_materials.insertObject();
   m_materialNames[nameString] = newIdx;

   return newIdx;
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
   if( formatString == "R32_UINT" )
   {
      return PixelFormat::R32_UINT;
   }
   if( formatString == "R16_UNORM" )
   {
      return PixelFormat::R16_UNORM;
   }
   if( formatString == "R8_UNORM" )
   {
      return PixelFormat::R8_UNORM;
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

static MaterialCache::TextureSlot StringToTextureSlot( const std::string& slotString )
{
   if( slotString == "ALBEDO" || slotString == "DIFFUSE" )
   {
      return MaterialCache::TextureSlot::ALBEDO;
   }
   if( slotString == "NORMAL" )
   {
      return MaterialCache::TextureSlot::NORMAL;
   }
   if( slotString == "METALNESS" )
   {
      return MaterialCache::TextureSlot::METALNESS;
   }
   if( slotString == "ROUGHNESS" || slotString == "GLOSSINESS" )
   {
      return MaterialCache::TextureSlot::ROUGHNESS;
   }
   if( slotString == "AMBIENT_OCCLUSION" )
   {
      return MaterialCache::TextureSlot::AMBIENT_OCCLUSION;
   }
   if( slotString == "DISPLACEMENT" )
   {
      return MaterialCache::TextureSlot::DISPLACEMENT;
   }

   CYD_ASSERT( !"Materials: Could not recognize string as a texture slot" );
   return MaterialCache::TextureSlot::ALBEDO;
}

static MaterialCache::TextureFallback StringToTextureFallback( const std::string& fallbackString )
{
   if( fallbackString == "BLACK" )
   {
      return MaterialCache::TextureFallback::BLACK;
   }
   if( fallbackString == "WHITE" )
   {
      return MaterialCache::TextureFallback::WHITE;
   }

   CYD_ASSERT( !"Materials: Could not recognize string as a texture fallback" );
   return MaterialCache::TextureFallback::BLACK;
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
         for( const auto& texture : *texturesIt )
         {
            const TextureSlot slot     = StringToTextureSlot( texture["SLOT"] );
            TextureEntry& textureEntry = material->textures[slot];

            std::string path       = "";
            const auto& pathIt     = texture.find( "PATH" );
            const auto& fallbackIt = texture.find( "FALLBACK" );

            if( pathIt != texture.end() )
            {
               path = std::string( MATERIAL_PATH );
               path += texture["PATH"];

               // This material has a path therefore it must be loaded from storage to VRAM
               material->needsLoadFromStorage = true;
            }

            if( fallbackIt != texture.end() )
            {
               textureEntry.fallback    = StringToTextureFallback( *fallbackIt );
               textureEntry.useFallback = true;
            }

            textureEntry.path                 = path;
            textureEntry.desc.format          = StringToPixelFormat( texture["FORMAT"] );
            textureEntry.desc.type            = StringToTextureType( texture["TYPE"] );
            textureEntry.desc.stages          = PipelineStage::FRAGMENT_STAGE;
            textureEntry.desc.usage           = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
            textureEntry.desc.generateMipmaps = true;

            if( textureEntry.desc.generateMipmaps )
            {
               textureEntry.desc.usage |= ImageUsage::TRANSFER_SRC;
            }
         }
      }

      m_materialNames[materialName] = index;

      printf( "Added material --> %s\n", materialName.c_str() );
   }

   printf( "======== End Static Materials ========\n" );

   return;
}
}