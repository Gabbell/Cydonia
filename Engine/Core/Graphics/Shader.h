#pragma once

#include <Core/Common/Include.h>

#include <vector>
#include <string>

// ================================================================================================
// Forwards
// ================================================================================================
FWDHANDLE( VkShaderModule );

namespace cyd
{
class Device;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class Shader
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
