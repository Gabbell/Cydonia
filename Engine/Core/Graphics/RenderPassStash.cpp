#include <Core/Graphics/RenderPassStash.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Device.h>

cyd::RenderPassStash::RenderPassStash( const Device& device ) : _device( device )
{
   _createDefaultRenderPasses();
}

const VkRenderPass cyd::RenderPassStash::findOrCreate( const RenderPassInfo& info )
{
   // Find
   const auto it = _renderPasses.find( info );
   if( it != _renderPasses.end() )
   {
      return it->second;
   }

   // Creating attachments
   std::vector<VkAttachmentDescription> attachmentDescs;
   std::vector<VkAttachmentReference> attachmentRefs;
   std::vector<VkSubpassDescription> subpassDescs;
   for( const auto& attachment : info.attachments )
   {
      VkAttachmentDescription vkAttachment = {};
      vkAttachment.format                  = cydFormatToVkFormat( attachment.format );
      vkAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
      vkAttachment.loadOp                  = cydOpToVkOp( attachment.loadOp );
      vkAttachment.storeOp                 = cydOpToVkOp( attachment.storeOp );
      vkAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      vkAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      vkAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;

      switch( attachment.usage )
      {
         case AttachmentUsage::UNKNOWN:
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
            break;
         case AttachmentUsage::COLOR:
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
         case AttachmentUsage::PRESENTATION:
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            break;
         case AttachmentUsage::TRANSFER_SRC:
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            break;
         case AttachmentUsage::TRANSFER_DST:
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            break;
         case AttachmentUsage::SHADER_READ:
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            break;
         default:
            CYDASSERT( !"RenderPass: Attachment usage not supported" )
            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_GENERAL;
      }
      attachmentDescs.push_back( std::move( vkAttachment ) );

      switch( attachment.type )
      {
         case AttachmentType::COLOR:
         {
            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment            = 0;
            colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass = {};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &colorAttachmentRef;

            attachmentRefs.push_back( std::move( colorAttachmentRef ) );
            subpassDescs.push_back( std::move( subpass ) );
            break;
         }
         default:
            CYDASSERT( !"RenderPass: Attachment type not supported" );
      }
   }

   VkRenderPassCreateInfo renderPassInfo = {};
   renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.attachmentCount        = static_cast<uint32_t>( attachmentDescs.size() );
   renderPassInfo.pAttachments           = attachmentDescs.data();
   renderPassInfo.subpassCount           = static_cast<uint32_t>( subpassDescs.size() );
   renderPassInfo.pSubpasses             = subpassDescs.data();

   // Creating render pass
   VkRenderPass renderPass;
   VkResult result =
       vkCreateRenderPass( _device.getVKDevice(), &renderPassInfo, nullptr, &renderPass );
   CYDASSERT( result == VK_SUCCESS && "RenderPass: Could not create default render pass" );

   _renderPasses.insert( {info, renderPass} );
   return renderPass;
}

void cyd::RenderPassStash::_createDefaultRenderPasses()
{
   // Color Presentation BGRA8_UNORM
   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.usage      = AttachmentUsage::PRESENTATION;

   RenderPassInfo info = {};
   info.attachments.push_back( colorPresentation );
   findOrCreate( info );
}

cyd::RenderPassStash::~RenderPassStash()
{
   for( const auto& renderPass : _renderPasses )
   {
      vkDestroyRenderPass( _device.getVKDevice(), renderPass.second, nullptr );
   }
}
