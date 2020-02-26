#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkDescriptorPool );
FWDHANDLE( VkDescriptorSet );
namespace vk
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class DescriptorPool final
{
  public:
   explicit DescriptorPool( const Device& device );
   ~DescriptorPool();

   VkDescriptorSet allocate( const cyd::DescriptorSetLayoutInfo& layout ) const;
   void free( const VkDescriptorSet& descSet ) const;

  private:
   const Device& m_device;

   VkDescriptorPool m_vkDescPool = nullptr;
};
}
