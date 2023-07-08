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
      TESS_CONTROL,
      TESS_EVAL,
      FRAGMENT,
      COMPUTE,
      UNKNOWN
   };

   Type getType() const { return m_type; }
   const VkShaderModule& getModule() const { return m_vkShader; }

  private:
   void _readShaderFile();
   void _createShaderModule();

   const Device& m_device;

   std::vector<char> m_byteCode;
   std::string m_shaderPath;
   Type m_type;

   VkShaderModule m_vkShader;
};
}
