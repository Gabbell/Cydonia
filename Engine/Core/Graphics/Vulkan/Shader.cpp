#include <Core/Graphics/Vulkan/Shader.h>

#include <Core/Common/Assert.h>
#include <Core/Common/Vulkan.h>

#include <Core/Graphics/Vulkan/Device.h>

#include <fstream>

cyd::Shader::Shader( const Device& device, const std::string& shaderPath )
    : _device( device ), _shaderPath( shaderPath )
{
   // Parsing shader type
   size_t dotIndex  = _shaderPath.find_last_of( "." );
   std::string type = _shaderPath.substr( dotIndex - 4, 4 );

   if( type == "vert" )
   {
      _type = Type::VERTEX;
   }
   else if( type == "frag" )
   {
      _type = Type::FRAGMENT;
   }
   else if( type == "comp" )
   {
      _type = Type::COMPUTE;
   }
   else
   {
      _type = Type::UNKNOWN;
   }

   // Creating shader
   _readShaderFile();
   _createShaderModule();
}

void cyd::Shader::_readShaderFile()
{
   std::ifstream shaderFile( _shaderPath, std::ios::ate | std::ios::binary );
   CYDASSERT( shaderFile.is_open() && "Shader: Could not open shader file" );

   size_t shaderSize = static_cast<size_t>( shaderFile.tellg() );

   _byteCode.resize( shaderSize );

   shaderFile.seekg( 0 );
   shaderFile.read( _byteCode.data(), shaderSize );
   shaderFile.close();
}

void cyd::Shader::_createShaderModule()
{
   VkShaderModuleCreateInfo createInfo = {};
   createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   createInfo.codeSize                 = _byteCode.size();
   createInfo.pCode                    = reinterpret_cast<const uint32_t*>( _byteCode.data() );

   VkResult result =
       vkCreateShaderModule( _device.getVKDevice(), &createInfo, nullptr, &_vkShader );
   CYDASSERT( result == VK_SUCCESS && "Shader: Could not create shader module" );
}

cyd::Shader::~Shader() { vkDestroyShaderModule( _device.getVKDevice(), _vkShader, nullptr ); }
