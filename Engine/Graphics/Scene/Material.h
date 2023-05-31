#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/Handles/ResourceHandle.h>

#include <string>

// ================================================================================================
// Definition
// ================================================================================================
/*
 * This is an instance of a material with optional GPU handles
 */
namespace CYD
{
struct MaterialDescription
{
   void addTexture( const TextureDescription& texDesc, const std::string& path = "" );

   static constexpr uint8_t MAX_TEXTURE_HANDLES = 8;
   static constexpr uint8_t MAX_BUFFER_HANDLES  = 8;

   std::array<TextureDescription, MAX_TEXTURE_HANDLES> textureDescs;
   std::array<std::string, MAX_TEXTURE_HANDLES> texturePaths;

   uint32_t textureCount = 0;
   uint32_t bufferCount  = 0;
};

class Material
{
  public:
   Material() = default;
   Material( const std::string& name, const MaterialDescription& desc )
       : m_name( name ), m_desc( desc )
   {
   }
   MOVABLE( Material );
   ~Material();

   const std::string& getName() const { return m_name; }

   void load( CmdListHandle cmdList );
   void unload();
   void bind( CmdListHandle cmdList, uint8_t set ) const;

   // For externally managed materials
   void updateTexture( TextureHandle texture, uint32_t binding );

  private:
   std::string m_name = "Unknown Material";

   // Resource Descriptions
   MaterialDescription m_desc;

   // GPU Resources
   std::array<TextureHandle, MaterialDescription::MAX_TEXTURE_HANDLES> m_textures = {};
   std::array<BufferHandle, MaterialDescription::MAX_BUFFER_HANDLES> m_buffers    = {};

   bool m_loaded = false;
};
}