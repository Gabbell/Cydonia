#include <ECS/Components/Rendering/SkyboxRenderableComponent.h>

#include <Graphics/RenderInterface.h>

namespace CYD
{
static constexpr char SKYBOX_PATH[] = "Assets/Textures/Skybox/";

bool SkyboxRenderableComponent::init( const std::string& skyboxName )
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   const uint32_t numberOfLayers = 6;

   TextureDescription texDesc = {};
   texDesc.width              = 1024;
   texDesc.height             = 1024;
   texDesc.size   = texDesc.width * texDesc.height * numberOfLayers * sizeof( uint32_t );
   texDesc.type   = ImageType::TEXTURE_2D;
   texDesc.layers = numberOfLayers;
   texDesc.format = PixelFormat::RGBA8_SRGB;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;
   texDesc.stages = ShaderStage::FRAGMENT_STAGE;

   const std::vector<std::string> paths = {SKYBOX_PATH + skyboxName + "lf.tga",
                                           SKYBOX_PATH + skyboxName + "rt.tga",
                                           SKYBOX_PATH + skyboxName + "dn.tga",
                                           SKYBOX_PATH + skyboxName + "up.tga",
                                           SKYBOX_PATH + skyboxName + "bk.tga",
                                           SKYBOX_PATH + skyboxName + "ft.tga"};

   cubemap = GRIS::CreateTexture( transferList, texDesc, paths );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   return true;
}

void SkyboxRenderableComponent::uninit() { GRIS::DestroyTexture( cubemap ); }

SkyboxRenderableComponent::~SkyboxRenderableComponent() { SkyboxRenderableComponent::uninit(); }
}
