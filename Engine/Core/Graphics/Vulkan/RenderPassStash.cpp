#include <Core/Graphics/Vulkan/RenderPassStash.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/TypeConversions.h>

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
   std::optional<VkAttachmentReference> depthRef;
   std::vector<VkAttachmentReference> colorRefs;
   std::vector<VkAttachmentDescription> attachmentDescs;
   std::vector<VkSubpassDescription> subpassDescs;
   std::vector<VkSubpassDependency> dependencies;
   for( const auto& attachment : info.attachments )
   {
      switch( attachment.type )
      {
         case AttachmentType::COLOR:
         {
            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment            = 0;
            colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorRefs.push_back( std::move( colorAttachmentRef ) );
            break;
         }
         case AttachmentType::DEPTH_STENCIL:
         {
            VkAttachmentReference depthAttachmentRef = {};
            depthAttachmentRef.attachment            = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthRef = depthAttachmentRef;
            break;
         }
         default:
            CYDASSERT( !"RenderPass: Attachment type not supported" );
      }

      VkAttachmentDescription vkAttachment = {};
      vkAttachment.format         = TypeConversions::cydFormatToVkFormat( attachment.format );
      vkAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
      vkAttachment.loadOp         = TypeConversions::cydOpToVkOp( attachment.loadOp );
      vkAttachment.storeOp        = TypeConversions::cydOpToVkOp( attachment.storeOp );
      vkAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      vkAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      vkAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
      vkAttachment.finalLayout =
          TypeConversions::cydImageLayoutToVKImageLayout( attachment.layout );

      attachmentDescs.push_back( std::move( vkAttachment ) );
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

      dependencies.push_back( std::move( dependency ) );
   }

   subpassDescs.push_back( std::move( subpass ) );

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
       vkCreateRenderPass( _device.getVKDevice(), &renderPassInfo, nullptr, &renderPass );
   CYDASSERT( result == VK_SUCCESS && "RenderPass: Could not create default render pass" );

   return _renderPasses.insert( { info, renderPass } ).first->second;
}

void cyd::RenderPassStash::_createDefaultRenderPasses()
{
   // Color Presentation BGRA8_UNORM
   Attachment colorPresentation = {};
   colorPresentation.format     = PixelFormat::BGRA8_UNORM;
   colorPresentation.loadOp     = LoadOp::CLEAR;
   colorPresentation.storeOp    = StoreOp::STORE;
   colorPresentation.type       = AttachmentType::COLOR;
   colorPresentation.layout     = ImageLayout::PRESENTATION;

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
