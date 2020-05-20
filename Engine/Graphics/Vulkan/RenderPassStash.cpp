#include <Graphics/Vulkan/RenderPassStash.h>

#include <Common/Assert.h>
#include <Common/Vulkan.h>

#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <optional>

namespace vk
{
RenderPassStash::RenderPassStash( const Device& device ) : m_device( device )
{
   _createDefaultRenderPasses();
}

VkRenderPass RenderPassStash::findOrCreate( const CYD::RenderPassInfo& info )
{
   // Find
   const auto it = m_renderPasses.find( info );
   if( it != m_renderPasses.end() )
   {
      return it->second;
   }

   // Creating attachments
   std::optional<VkAttachmentReference> depthRef;
   std::vector<VkAttachmentReference> colorRefs;
   std::vector<VkAttachmentDescription> attachmentDescs;
   std::vector<VkSubpassDescription> subpassDescs;
   std::vector<VkSubpassDependency> dependencies;
   for( const auto& attachment : info.attachments )
   {
      VkAttachmentDescription vkAttachment = {};
      vkAttachment.format                  = TypeConversions::cydToVkFormat( attachment.format );
      vkAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
      vkAttachment.loadOp                  = TypeConversions::cydToVkOp( attachment.loadOp );
      vkAttachment.storeOp                 = TypeConversions::cydToVkOp( attachment.storeOp );
      vkAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      vkAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      vkAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;

      switch( attachment.type )
      {
         case CYD::AttachmentType::COLOR_PRESENTATION:
         {
            VkAttachmentReference presentationAttachmentRef = {};
            presentationAttachmentRef.attachment            = 0;
            presentationAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorRefs.push_back( presentationAttachmentRef );

            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            break;
         }
         case CYD::AttachmentType::COLOR:
         {
            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment            = 0;
            colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorRefs.push_back( colorAttachmentRef );

            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            break;
         }
         case CYD::AttachmentType::DEPTH_STENCIL:
         {
            VkAttachmentReference depthAttachmentRef = {};
            depthAttachmentRef.attachment            = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthRef = depthAttachmentRef;

            vkAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            break;
         }
         default:
            CYDASSERT( !"RenderPass: Attachment type not supported" );
      }

      attachmentDescs.push_back( vkAttachment );
   }

   VkSubpassDescription subpass = {};
   subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount = static_cast<uint32_t>( colorRefs.size() );
   subpass.pColorAttachments    = colorRefs.data();

   if( depthRef.has_value() )
   {
      // We have a depth attachment reference
      subpass.pDepthStencilAttachment = &depthRef.value();

      VkSubpassDependency dependency = {};
      dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
      dependency.dstSubpass          = 0;
      dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.srcAccessMask       = 0;
      dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      dependency.dstAccessMask =
          VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

      dependencies.push_back( dependency );
   }

   subpassDescs.push_back( subpass );

   VkRenderPassCreateInfo renderPassInfo = {};
   renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassInfo.attachmentCount        = static_cast<uint32_t>( attachmentDescs.size() );
   renderPassInfo.pAttachments           = attachmentDescs.data();
   renderPassInfo.subpassCount           = static_cast<uint32_t>( subpassDescs.size() );
   renderPassInfo.pSubpasses             = subpassDescs.data();
   renderPassInfo.dependencyCount        = static_cast<uint32_t>( dependencies.size() );
   renderPassInfo.pDependencies          = dependencies.data();

   // Creating render pass
   VkRenderPass renderPass;
   VkResult result =
       vkCreateRenderPass( m_device.getVKDevice(), &renderPassInfo, nullptr, &renderPass );
   CYDASSERT( result == VK_SUCCESS && "RenderPass: Could not create default render pass" );

   return m_renderPasses.insert( { info, renderPass } ).first->second;
}

void RenderPassStash::_createDefaultRenderPasses()
{
   //
}

RenderPassStash::~RenderPassStash()
{
   for( const auto& renderPass : m_renderPasses )
   {
      vkDestroyRenderPass( m_device.getVKDevice(), renderPass.second, nullptr );
   }
}
}
