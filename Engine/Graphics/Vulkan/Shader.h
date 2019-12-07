#pragma once

#include <Common/Include.h>

#include <string>
#include <vector>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkShaderModule );

namespace vk
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class Shader final
{
  public:
   Shader( const Device& device, const std::string& shaderName );
   ~Shader();

   enum class Type
   {
      VERTEX,
      FRAGMENT,
      COMPUTE,
      UNKNOWN
   };

   Type getType() const { return _type; }
   const VkShaderModule& getModule() const { return _vkShader; }

  private:
   void _readShaderFile();
   void _createShaderModule();

   const Device& _device;

   std::vector<char> _byteCode;
   std::string _shaderPath;
   Type _type;

   VkShaderModule _vkShader;
};
}
