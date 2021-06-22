#include <Graphics/Vulkan/DebugUtilsLabel.h>

#include <Common/Vulkan.h>

#include <Graphics/Vulkan/CommandBuffer.h>
#include <Graphics/Vulkan/Device.h>

namespace vk::DebugUtilsLabel
{
PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel   = VK_NULL_HANDLE;
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel       = VK_NULL_HANDLE;
PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabel = VK_NULL_HANDLE;

void Initialize( const Device& device )
{
   vkCmdBeginDebugUtilsLabel = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetDeviceProcAddr(
       device.getVKDevice(), "vkCmdBeginDebugUtilsLabelEXT" );
   vkCmdEndDebugUtilsLabel = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetDeviceProcAddr(
       device.getVKDevice(), "vkCmdEndDebugUtilsLabelEXT" );
   vkCmdInsertDebugUtilsLabel = (PFN_vkCmdInsertDebugUtilsLabelEXT)vkGetDeviceProcAddr(
       device.getVKDevice(), "vkCmdInsertDebugUtilsLabelEXT" );
}

void Begin( const CommandBuffer& cmdBuffer, const char* name, std::array<float, 4> color )
{
   VkDebugUtilsLabelEXT markerInfo;
   markerInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
   markerInfo.pNext      = nullptr;
   markerInfo.pLabelName = name;
   memcpy( markerInfo.color, color.data(), sizeof( markerInfo.color ) );

   vkCmdBeginDebugUtilsLabel( cmdBuffer.getVKBuffer(), &markerInfo );
}

void End( const CommandBuffer& cmdBuffer ) { vkCmdEndDebugUtilsLabel( cmdBuffer.getVKBuffer() ); }

void Insert( const CommandBuffer& cmdBuffer, const char* name, std::array<float, 4> color )
{
   VkDebugUtilsLabelEXT markerInfo;
   markerInfo.sType      = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
   markerInfo.pNext      = nullptr;
   markerInfo.pLabelName = name;
   memcpy( markerInfo.color, color.data(), sizeof( markerInfo.color ) );

   vkCmdInsertDebugUtilsLabel( cmdBuffer.getVKBuffer(), &markerInfo );
}
}