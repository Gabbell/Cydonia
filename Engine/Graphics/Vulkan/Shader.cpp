#include <Graphics/Vulkan/Shader.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan.h>
#include <Graphics/Vulkan/Device.h>

#include <fstream>

namespace vk
{
Shader::Shader( const Device& device, const std::string& shaderPath )
    : m_device( device ), m_shaderPath( shaderPath )
{
   // Parsing shader type
   size_t dotIndex  = m_shaderPath.find_last_of( "." );
   std::string type = m_shaderPath.substr( dotIndex - 4, 4 );

   if( type == "VERT" )
   {
      m_type = Type::VERTEX;
   }
   else if( type == "FRAG" )
   {
      m_type = Type::FRAGMENT;
   }
   else if( type == "COMP" )
   {
      m_type = Type::COMPUTE;
   }
   else
   {
      m_type = Type::UNKNOWN;
   }

   // Creating shader
   _readShaderFile();
   _createShaderModule();
}

void Shader::_readShaderFile()
{
   std::ifstream shaderFile( m_shaderPath, std::ios::ate | std::ios::binary );
   CYD_ASSERT( shaderFile.is_open() && "Shader: Could not open shader file" );

   size_t shaderSize = static_cast<size_t>( shaderFile.tellg() );

   m_byteCode.resize( shaderSize );

   shaderFile.seekg( 0 );
   shaderFile.read( m_byteCode.data(), shaderSize );
   shaderFile.close();
}

void Shader::_createShaderModule()
{
   VkShaderModuleCreateInfo createInfo = {};
   createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize                 = m_byteCode.size();
   createInfo.pCode                    = reinterpret_cast<const uint32_t*>( m_byteCode.data() );

   VkResult result =
       vkCreateShaderModule( m_device.getVKDevice(), &createInfo, nullptr, &m_vkShader );
   CYD_ASSERT( result == VK_SUCCESS && "Shader: Could not create shader module" );
}

Shader::~Shader() { vkDestroyShaderModule( m_device.getVKDevice(), m_vkShader, nullptr ); }
}