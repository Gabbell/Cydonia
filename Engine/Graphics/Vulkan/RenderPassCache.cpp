#include <Graphics/Vulkan/RenderPassCache.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/Synchronization.h>
#include <Graphics/Vulkan/Device.h>
#include <Graphics/Vulkan/TypeConversions.h>

#include <optional>

namespace vk
{
RenderPassCache::RenderPassCache( const Device& device ) : m_device( device )
{
   _createDefaultRenderPasses();
}

VkRenderPass RenderPassCache::findOrCreate( const RenderPassInfo& targetsInfo )
{
   // Find
   const auto it = m_renderPasses.find( targetsInfo );
   if( it != m_renderPasses.end() )
   {
      return it->second;
   }

   // Creating attachments
   std::vector<VkAttachmentReference> colorRefs;
   std::vector<VkAttachmentReference> depthRefs;

   std::vector<VkAttachmentDescription> attachmentDescs;
   std::vector<VkSubpassDescription> subpassDescs;
   std::vector<VkSubpassDependency> dependencies;

   uint32_t attachmentIdx = 0;
   for( const auto& attachment : targetsInfo.attachments )
   {
      VkAttachmentDescription vkAttachment = {};
      vkAttachment.format                  = TypeConversions::cydToVkFormat( attachment.format );
      vkAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
      vkAttachment.loadOp                  = TypeConversions::cydToVkOp( attachment.loadOp );
      vkAttachment.storeOp                 = TypeConversions::cydToVkOp( attachment.storeOp );
      vkAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
      vkAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
      vkAttachment.initialLayout = Synchronization::GetLayoutFromAccess( attachment.initialAccess );
      vkAttachment.finalLayout   = Synchronization::GetLayoutFromAccess( attachment.nextAccess );

      switch( attachment.type )
      {
         case CYD::AttachmentType::COLOR_PRESENTATION:
         {
            VkAttachmentReference presentationAttachmentRef = {};
            presentationAttachmentRef.attachment            = attachmentIdx;
            presentationAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorRefs.push_back( presentationAttachmentRef );
            attachmentIdx++;
            break;
         }
         case CYD::AttachmentType::COLOR:
         {
            VkAttachmentReference colorAttachmentRef = {};
            colorAttachmentRef.attachment            = attachmentIdx;
            colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            colorRefs.push_back( colorAttachmentRef );
            attachmentIdx++;
            break;
         }
         case CYD::AttachmentType::DEPTH_STENCIL:
         {
            VkAttachmentReference depthAttachmentRef = {};
            depthAttachmentRef.attachment            = attachmentIdx;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            depthRefs.push_back( depthAttachmentRef );
            attachmentIdx++;
            break;
         }
         case CYD::AttachmentType::DEPTH:
         {
            // Needs VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures
            VkAttachmentReference depthAttachmentRef = {};
            depthAttachmentRef.attachment            = attachmentIdx;
            depthAttachmentRef.layout                = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;

            depthRefs.push_back( depthAttachmentRef );
            attachmentIdx++;
            break;
         }
         default:
            CYD_ASSERT( !"RenderPass: Attachment type not supported" );
      }

      attachmentDescs.push_back( vkAttachment );
   }

   VkSubpassDescription& subpass = subpassDescs.emplace_back();
   subpass.pipelineBindPoint     = VK_PIPELINE_BIND_POINT_GRAPHICS;
   subpass.colorAttachmentCount  = static_cast<uint32_t>( colorRefs.size() );
   subpass.pColorAttachments     = colorRefs.data();

   if( !depthRefs.empty() )
   {
      CYD_ASSERT( depthRefs.size() == 1 && "Multi depth attachment not supported" );

      // We have a depth attachment reference
      subpass.pDepthStencilAttachment = &depthRefs[0];
   }

   // Very generous dependency
   VkSubpassDependency& dependency = dependencies.emplace_back();
   dependency.srcSubpass           = VK_SUBPASS_EXTERNAL;
   dependency.dstSubpass           = 0;
   dependency.dependencyFlags      = VK_DEPENDENCY_BY_REGION_BIT;
   dependency.srcStageMask =
       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
   dependency.dstStageMask =
       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
   dependency.srcAccessMask = 0;
   dependency.dstAccessMask =
       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

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
   CYD_ASSERT( result == VK_SUCCESS && "RenderPass: Could not create default render pass" );

   return m_renderPasses.insert( { targetsInfo, renderPass } ).first->second;
}

void RenderPassCache::_createDefaultRenderPasses()
{
   //
}

RenderPassCache::~RenderPassCache()
{
   for( const auto& renderPass : m_renderPasses )
   {
      vkDestroyRenderPass( m_device.getVKDevice(), renderPass.second, nullptr );
   }
}
}
