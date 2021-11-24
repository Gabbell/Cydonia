#pragma once

#include <memory>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
namespace vk
{
class Device;
class Shader;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace vk
{
class ShaderCache final
{
  public:
   explicit ShaderCache( const Device& device );
   ~ShaderCache();

   // Shader names are unique. Shader types are inferred from file name
   const Shader* getShader( const std::string& shaderName );

  private:
   void _initializeAllShaders();

   const Device& _device;

   std::unordered_map<std::string, std::unique_ptr<Shader>> _shaders;
};
}
