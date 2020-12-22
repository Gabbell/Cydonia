#pragma once

#include <Common/Include.h>

#include <Graphics/Handles/ResourceHandle.h>

#include <cstdint>
#include <string_view>
#include <unordered_map>

namespace CYD
{
struct Material
{
   ~Material();
   TextureHandle albedo;     // Diffuse/Albedo color map
   TextureHandle normals;    // Normal map
   TextureHandle metalness;  // Metallic/Specular map
   TextureHandle roughness;  // Roughness map
   TextureHandle ao;         // Ambient occlusion map
   TextureHandle height;     // Height map
};

struct Mesh
{
   ~Mesh();
   VertexBufferHandle vertexBuffer;
   IndexBufferHandle indexBuffer;
   uint32_t vertexCount = 0;
   uint32_t indexCount  = 0;
};

// For read-only assets loaded from disk
class AssetStash final
{
  public:
   AssetStash() = default;
   NON_COPIABLE( AssetStash );
   virtual ~AssetStash() = default;

   // void cleanup();

   // High-level asset loading
   const Material& loadMaterial( CmdListHandle transferList, const std::string_view materialPath );
   const Mesh& loadMesh( CmdListHandle transferList, const std::string_view meshPath );

  private:
   static constexpr uint32_t INITIAL_AMOUNT_RESOURCES = 128;

   std::unordered_map<std::string, Material> m_materials;
   std::unordered_map<std::string, Mesh> m_meshes;
};
}
