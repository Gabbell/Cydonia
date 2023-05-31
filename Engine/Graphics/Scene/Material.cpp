#include <Graphics/Scene/Material.h>

#include <Common/Assert.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
void MaterialDescription::addTexture( const TextureDescription& texDesc, const std::string& path )
{
   textureDescs[textureCount] = texDesc;
   texturePaths[textureCount] = path;
   textureCount++;
}

void Material::load( CmdListHandle cmdList )
{
   if( m_loaded )
   {
      return;
   }

   for( uint32_t i = 0; i < m_desc.textureCount; ++i )
   {
      // If we have a path, load from storage. If not, this texture is probably managed elsewhere
      if( !m_desc.texturePaths[i].empty() )
      {
         m_textures[i] = GRIS::CreateTexture( cmdList, m_desc.textureDescs[i], m_desc.texturePaths[i] );
      }
   }

   m_loaded = true;
}

void Material::unload()
{
   for( uint32_t i = 0; i < m_desc.textureCount; ++i )
   {
      GRIS::DestroyTexture( m_textures[i] );
   }

   for( uint32_t i = 0; i < m_desc.bufferCount; ++i )
   {
      GRIS::DestroyBuffer( m_buffers[i] );
   }

   m_loaded = false;
}

void Material::bind( CmdListHandle cmdList, uint8_t set ) const
{
   for( uint32_t i = 0; i < m_desc.textureCount; ++i )
   {
      // Set is hardcoded to 1 for materials right now
      // TODO Allow non-contiguous bindings?
      GRIS::BindTexture( cmdList, m_textures[i], i, set );
   }
}

void Material::updateTexture(TextureHandle texture, uint32_t binding)
{
   CYDASSERT( binding < MaterialDescription::MAX_TEXTURE_HANDLES );

   if( !m_textures[binding] ) m_desc.textureCount++;
   m_textures[binding] = texture;
}

Material::~Material() { unload(); }
}