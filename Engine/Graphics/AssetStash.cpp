#include <Graphics/AssetStash.h>

#include <Common/Assert.h>

#include <Graphics/GraphicsTypes.h>
#include <Graphics/VertexLayout.h>
#include <Graphics/GRIS/RenderInterface.h>
#include <Graphics/Utility/GraphicsIO.h>

namespace CYD
{
// Asset Paths Constants
// ================================================================================================
static constexpr char MATERIAL_PATH[] = "Data/Materials/";
static constexpr char MESH_PATH[]     = "Data/Meshes/";

static constexpr char ALBEDO_NAME[]       = "albedo.png";
static constexpr char NORMALS_NAME[]      = "normals.png";
static constexpr char DISPLACEMENT_NAME[] = "displacement.png";
static constexpr char METALNESS_NAME[]    = "metalness.png";
static constexpr char ROUGHNESS_NAME[]    = "roughness.png";
static constexpr char AO_NAME[]           = "ao.png";

const Mesh& AssetStash::getMesh( const std::string_view name )
{
   const std::string meshString( name );
   auto it = m_meshes.find( meshString );
   if( it != m_meshes.end() )
   {
      return it->second;
   }

   return m_meshes[""];
}

const Material& AssetStash::getMaterial( const std::string_view name )
{
   const std::string materialString( name );
   auto it = m_materials.find( materialString );
   if( it != m_materials.end() )
   {
      return it->second;
   }

   return m_materials[""];
}

bool AssetStash::loadMaterialFromPath(
    CmdListHandle transferList,
    const std::string_view materialPath )
{
   const std::string materialString( materialPath );
   auto it = m_materials.find( materialString );
   if( it == m_materials.end() )
   {
      TextureDescription rgbaDesc = {};
      rgbaDesc.type               = ImageType::TEXTURE_2D;
      rgbaDesc.format             = PixelFormat::RGBA8_SRGB;
      rgbaDesc.usage              = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
      rgbaDesc.stages             = ShaderStage::FRAGMENT_STAGE;
      rgbaDesc.name               = materialPath;

      TextureDescription rDesc = rgbaDesc;
      rDesc.format             = PixelFormat::R32F;

      // TODO More dynamic resource loading. Maybe depending on the pipeline, load only what we need
      const std::string fullPath = MATERIAL_PATH + materialString + "/";

      const std::string albedoPath    = fullPath + ALBEDO_NAME;
      const std::string normalsPath   = fullPath + NORMALS_NAME;
      const std::string metalnessPath = fullPath + METALNESS_NAME;
      const std::string roughnessPath = fullPath + ROUGHNESS_NAME;
      const std::string aoPath        = fullPath + AO_NAME;
      const std::string dispPath      = fullPath + DISPLACEMENT_NAME;

      Material& material = m_materials[materialString];

      material.albedo    = GRIS::CreateTexture( transferList, rgbaDesc, albedoPath );
      material.normals   = GRIS::CreateTexture( transferList, rgbaDesc, normalsPath );
      material.disp      = GRIS::CreateTexture( transferList, rgbaDesc, dispPath );
      material.metalness = GRIS::CreateTexture( transferList, rDesc, metalnessPath );
      material.roughness = GRIS::CreateTexture( transferList, rDesc, roughnessPath );
      material.ao        = GRIS::CreateTexture( transferList, rDesc, aoPath );

      return true;
   }

   return false;
}

bool AssetStash::loadMeshFromPath( CmdListHandle transferList, const std::string_view meshPath )
{
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
          vertices.data(),
          meshPath );

      mesh.vertexCount = static_cast<uint32_t>( vertices.size() );

      mesh.indexBuffer = GRIS::CreateIndexBuffer(
          transferList, static_cast<uint32_t>( indices.size() ), indices.data(), meshPath );

      mesh.indexCount = static_cast<uint32_t>( indices.size() );

      return true;
   }

   return false;
}

bool AssetStash::loadMesh(
    CmdListHandle transferList,
    const std::string_view name,
    const std::vector<Vertex>& vertices,
    const std::vector<uint32_t>& indices )
{
   const std::string meshString( name );
   auto it = m_meshes.find( meshString );
   if( it == m_meshes.end() )
   {
      Mesh& mesh = m_meshes[meshString];

      mesh.vertexBuffer = GRIS::CreateVertexBuffer(
          transferList,
          static_cast<uint32_t>( vertices.size() ),
          static_cast<uint32_t>( sizeof( Vertex ) ),
          vertices.data(),
          name );

      mesh.vertexCount = static_cast<uint32_t>( vertices.size() );

      mesh.indexBuffer = GRIS::CreateIndexBuffer(
          transferList, static_cast<uint32_t>( indices.size() ), indices.data(), name );

      mesh.indexCount = static_cast<uint32_t>( indices.size() );

      return true;
   }

   return false;
}

Mesh::~Mesh()
{
   GRIS::DestroyBuffer( vertexBuffer );
   GRIS::DestroyBuffer( indexBuffer );
}
}