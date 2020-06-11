#include <ECS/Components/Rendering/PhongRenderableComponent.h>

#include <Graphics/RenderInterface.h>
#include <Graphics/GraphicsIO.h>

namespace CYD
{
// Placeholder texture
// TODO move to a place for static resources
static constexpr uint32_t placeholderData[] = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };
static constexpr TextureDescription placeholderDesc = {
    sizeof( placeholderData ),
    2,  // width
    2,  // height
    1,
    ImageType::TEXTURE_2D,
    PixelFormat::BGRA8_UNORM,
    ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED,
    ShaderStage::FRAGMENT_STAGE};

bool PhongRenderableComponent::init()
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   GRIS::StartRecordingCommandList( transferList );

   texture = GRIS::CreateTexture( transferList, placeholderDesc, placeholderData );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   return true;
}

void PhongRenderableComponent::uninit() { GRIS::DestroyTexture( texture ); }

PhongRenderableComponent::~PhongRenderableComponent() { PhongRenderableComponent::uninit(); }

}
