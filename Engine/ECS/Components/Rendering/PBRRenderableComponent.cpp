#include <ECS/Components/Rendering/PBRRenderableComponent.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/GraphicsIO.h>

namespace CYD
{
static const std::string PBR_PATH = "Assets/Textures/PBR/";

PBRRenderableComponent::PBRRenderableComponent() : RenderableComponent( RenderableType::PBR ) {}

bool PBRRenderableComponent::init( const std::string& modelName )
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   // TODO don't use rgba8_srgb for every texture
   TextureDescription texDesc = {};
   texDesc.width              = 4096;
   texDesc.height             = 4096;
   texDesc.size               = texDesc.width * texDesc.height * sizeof( uint32_t );
   texDesc.type               = ImageType::TEXTURE_2D;
   texDesc.format             = PixelFormat::RGBA8_SRGB;
   texDesc.usage              = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
   texDesc.stages             = ShaderStage::FRAGMENT_STAGE;

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

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   return true;
}

void PBRRenderableComponent::uninit()
{
   GRIS::DestroyTexture( albedo );
   GRIS::DestroyTexture( normalMap );
   GRIS::DestroyTexture( metalnessMap );
   GRIS::DestroyTexture( roughnessMap );
   GRIS::DestroyTexture( ambientOcclusionMap );
}

PBRRenderableComponent::~PBRRenderableComponent() { PBRRenderableComponent::uninit(); }
}
