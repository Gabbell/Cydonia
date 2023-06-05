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
class Material
{
  public:
   enum TextureSlot
   {
      ALBEDO            = 0,
      DIFFUSE           = 0,
      NORMALS           = 1,
      METALNESS         = 2,
      ROUGHNESS         = 3,
      GLOSSINESS        = 3,
      AMBIENT_OCCLUSION = 4,
      DISPLACEMENT      = 5,
      COUNT
   };

   struct Description
   {
      void addTexture( const TextureDescription& texDesc, const std::string& path = "" );

      static constexpr uint8_t MAX_BUFFER_HANDLES = 8;

      std::array<TextureDescription, TextureSlot::COUNT> textureDescs;
      std::array<std::string, TextureSlot::COUNT> texturePaths;

      uint32_t textureCount = 0;
      uint32_t bufferCount  = 0;
   };

   Material() = default;
   Material( const std::string& name, const Description& desc ) : m_name( name ), m_desc( desc ) {}
   MOVABLE( Material );
   ~Material();

   const std::string& getName() const { return m_name; }

   void load( CmdListHandle cmdList );
   void unload();
   void bind( CmdListHandle cmdList, uint8_t set ) const;

   // For externally managed materials
   void updateTexture( TextureHandle texture, TextureSlot slot );

  private:
   std::string m_name = "Unknown Material";

   // Resource Descriptions
   Description m_desc;

   // GPU Resources
   std::array<TextureHandle, TextureSlot::COUNT> m_textures = {};
   std::array<BufferHandle, TextureSlot::COUNT> m_buffers   = {};

   bool m_loaded = false;
};
}