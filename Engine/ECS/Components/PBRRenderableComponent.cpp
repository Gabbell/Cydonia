#include <ECS/Components/PBRRenderableComponent.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/GraphicsIO.h>

namespace cyd
{
static const std::string PBR_PATH  = "Assets/Textures/PBR/";
static const std::string MESH_PATH = "Assets/Meshes/";

PBRRenderableComponent::PBRRenderableComponent( const std::string& modelName )
    : RenderableComponent( RenderableType::PBR )
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   std::vector<Vertex> vertices;
   std::vector<uint32_t> indices;
   GraphicsIO::LoadMesh( MESH_PATH + modelName, vertices, indices );

   GRIS::StartRecordingCommandList( transferList );

   TextureDescription texDesc = {};
   texDesc.format             = PixelFormat::RGBA8_SRGB;
   texDesc.usage              = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
   texDesc.type               = ImageType::TEXTURE_2D;
   texDesc.width              = 2048;
   texDesc.height             = 2048;
   texDesc.size               = texDesc.width * texDesc.height * sizeof( uint32_t );

   const std::string modelPath = PBR_PATH + modelName + "/";

   const std::string albedoPath    = modelPath + "albedo.png";
   const std::string normalPath    = modelPath + "normal.png";
   const std::string metalnessPath = modelPath + "metalness.png";
   const std::string roughnessPath = modelPath + "roughness.png";
   const std::string aoPath        = modelPath + "ao.png";
   const std::string heightPath    = modelPath + "height.png";

   albedo              = GRIS::CreateTexture( transferList, texDesc, albedoPath );
   normalMap           = GRIS::CreateTexture( transferList, texDesc, normalPath );
   metalnessMap        = GRIS::CreateTexture( transferList, texDesc, metalnessPath );
   roughnessMap        = GRIS::CreateTexture( transferList, texDesc, roughnessPath );
   ambientOcclusionMap = GRIS::CreateTexture( transferList, texDesc, aoPath );
   heightMap           = GRIS::CreateTexture( transferList, texDesc, heightPath );

   vertexBuffer = GRIS::CreateVertexBuffer(
       transferList,
       static_cast<uint32_t>( vertices.size() ),
       static_cast<uint32_t>( sizeof( Vertex ) ),
       vertices.data() );

   vertexCount = static_cast<uint32_t>( vertices.size() );

   indexBuffer = GRIS::CreateIndexBuffer(
       transferList, static_cast<uint32_t>( indices.size() ), indices.data() );

   indexCount = static_cast<uint32_t>( indices.size() );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );
}

void PBRRenderableComponent::uninit()
{
   GRIS::DestroyTexture( albedo );
   GRIS::DestroyTexture( normalMap );
   GRIS::DestroyTexture( metalnessMap );
   GRIS::DestroyTexture( roughnessMap );
   GRIS::DestroyTexture( ambientOcclusionMap );
   GRIS::DestroyIndexBuffer( indexBuffer );
   GRIS::DestroyVertexBuffer( vertexBuffer );
}
}
