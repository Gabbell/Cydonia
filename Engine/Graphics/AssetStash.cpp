#include <Graphics/AssetStash.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/GraphicsIO.h>

namespace CYD
{
// Asset Paths Constants
// ================================================================================================
static const std::string MATERIAL_PATH = "Data/Materials/";
static const std::string MESH_PATH     = "Data/Meshes/";

static const std::string ALBEDO_NAME    = "albedo.png";
static const std::string NORMALS_NAME   = "normals.png";
static const std::string METALNESS_NAME = "metalness.png";
static const std::string ROUGHNESS_NAME = "roughness.png";
static const std::string AO_NAME        = "ao.png";
static const std::string HEIGHT_NAME    = "height.png";

const Material& AssetStash::loadMaterial(
    CmdListHandle transferList,
    const std::string_view materialPath )
{
   if( materialPath.empty() )
   {
      return m_materials[""];
   }

   const std::string materialString( materialPath );

   auto it = m_materials.find( materialString );
   if( it == m_materials.end() )
   {
      TextureDescription texDesc = {};
      texDesc.width              = 2048;
      texDesc.height             = 2048;
      texDesc.size               = texDesc.width * texDesc.height * sizeof( uint32_t );
      texDesc.type               = ImageType::TEXTURE_2D;
      texDesc.format             = PixelFormat::RGBA8_SRGB;
      texDesc.usage              = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
      texDesc.stages             = ShaderStage::FRAGMENT_STAGE;

      // TODO More dynamic resource loading. Maybe depending on the pipeline, load only what we need
      const std::string fullPath = MATERIAL_PATH + materialString + "/";

      const std::string albedoPath    = fullPath + ALBEDO_NAME;
      const std::string normalsPath   = fullPath + NORMALS_NAME;
      const std::string metalnessPath = fullPath + METALNESS_NAME;
      const std::string roughnessPath = fullPath + ROUGHNESS_NAME;
      const std::string aoPath        = fullPath + AO_NAME;
      const std::string heightPath    = fullPath + HEIGHT_NAME;

      Material& material = m_materials[materialString];

      material.albedo    = GRIS::CreateTexture( transferList, texDesc, albedoPath );
      material.normals   = GRIS::CreateTexture( transferList, texDesc, normalsPath );
      material.height    = GRIS::CreateTexture( transferList, texDesc, heightPath );
      material.metalness = GRIS::CreateTexture( transferList, texDesc, metalnessPath );
      material.roughness = GRIS::CreateTexture( transferList, texDesc, roughnessPath );
      material.ao        = GRIS::CreateTexture( transferList, texDesc, aoPath );

      return material;
   }

   return m_materials[""];
}

const Mesh& AssetStash::loadMesh( CmdListHandle transferList, const std::string_view meshPath )
{
   if( meshPath.empty() )
   {
      return m_meshes[""];
   }

   const std::string meshString( meshPath );

   auto it = m_meshes.find( meshString );
   if( it == m_meshes.end() )
   {
      // Mesh was not previously loaded, load it
      const std::string fullPath = MESH_PATH + meshString + "/";

      std::vector<Vertex> vertices;
      std::vector<uint32_t> indices;
      GraphicsIO::LoadMesh( meshString, vertices, indices );

      Mesh& mesh = m_meshes[meshString];

      mesh.vertexBuffer = GRIS::CreateVertexBuffer(
          transferList,
          static_cast<uint32_t>( vertices.size() ),
          static_cast<uint32_t>( sizeof( Vertex ) ),
          vertices.data() );

      mesh.vertexCount = static_cast<uint32_t>( vertices.size() );

      mesh.indexBuffer = GRIS::CreateIndexBuffer(
          transferList, static_cast<uint32_t>( indices.size() ), indices.data() );

      mesh.indexCount = static_cast<uint32_t>( indices.size() );

      return mesh;
   }

   return m_meshes[""];
}

Material::~Material()
{
   GRIS::DestroyTexture( albedo );
   GRIS::DestroyTexture( normals );
   GRIS::DestroyTexture( height );
   GRIS::DestroyTexture( metalness );
   GRIS::DestroyTexture( roughness );
   GRIS::DestroyTexture( ao );
}

Mesh::~Mesh()
{
   GRIS::DestroyBuffer( vertexBuffer );
   GRIS::DestroyBuffer( indexBuffer );
}
}