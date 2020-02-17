#include <ECS/Components/RenderableComponent.h>

#include <Graphics/RenderInterface.h>

#include <array>

#include <glm/glm.hpp>

namespace cyd
{
bool RenderableComponent::init()
{
   const CmdListHandle transferList = GRIS::CreateCommandList( TRANSFER );

   matBuffer = GRIS::CreateUniformBuffer( sizeof( glm::vec4 ) );

   // Triangle
   const std::vector<Vertex> vertices = {
       {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
       {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
       {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
       {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
       {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{-1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{-1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
       {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
       {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
       {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{-1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}},
       {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}},
       {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
       {{-1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
       {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.0f}}};

   // Placeholder texture
   std::array<uint32_t, 4> texData = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000, 0xFFFFFFFF};
   const size_t texSize            = sizeof( texData[0] ) * texData.size();

   TextureDescription texDesc;
   texDesc.size   = texSize;
   texDesc.width  = 2;
   texDesc.height = 2;
   texDesc.type   = ImageType::TEXTURE_2D;
   texDesc.format = PixelFormat::BGRA8_UNORM;
   texDesc.usage  = ImageUsage::TRANSFER_DST | ImageUsage::SAMPLED;

   GRIS::StartRecordingCommandList( transferList );

   matTexture = GRIS::CreateTexture( transferList, texDesc, texData.data() );

   vertexBuffer = GRIS::CreateVertexBuffer(
       transferList,
       static_cast<uint32_t>( vertices.size() ),
       static_cast<uint32_t>( sizeof( Vertex ) ),
       vertices.data() );

   GRIS::EndRecordingCommandList( transferList );

   GRIS::SubmitCommandList( transferList );
   GRIS::WaitOnCommandList( transferList );
   GRIS::DestroyCommandList( transferList );

   return true;
}

void RenderableComponent::uninit()
{
   GRIS::DestroyUniformBuffer( matBuffer );
   GRIS::DestroyIndexBuffer( indexBuffer );
   GRIS::DestroyVertexBuffer( vertexBuffer );
   GRIS::DestroyTexture( matTexture );
}

RenderableComponent::~RenderableComponent() = default;
}
