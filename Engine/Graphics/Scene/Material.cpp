#include <Graphics/Scene/Material.h>

#include <Common/Assert.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
void Material::Description::addTexture( const TextureDescription& texDesc, const std::string& path )
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
   for( uint32_t i = 0; i < m_textures.size(); ++i )
   {
      // Set is hardcoded to 1 for materials right now
      // TODO Allow non-contiguous bindings?
      if( m_textures[i] )
      {
         GRIS::BindTexture( cmdList, m_textures[i], i, set );
      }
   }
}

void Material::updateTexture(TextureHandle texture, TextureSlot slot)
{
   if( !m_textures[slot] ) m_desc.textureCount++;
   m_textures[slot] = texture;
}

Material::~Material() { unload(); }
}