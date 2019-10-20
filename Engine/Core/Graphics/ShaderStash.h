#pragma once

#include <memory>
#include <unordered_map>

// ================================================================================================
// Forwards
// ================================================================================================
namespace cyd
{
class Device;
class Shader;
}

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
class ShaderStash
{
  public:
   explicit ShaderStash( const Device& device );
   ~ShaderStash();

   // Shader names are unique. Shader types are inferred from file name
   const Shader* getShader( std::string shaderName );

  private:
   void _initializeAllShaders();

   const Device& _device;

   std::unordered_map<std::string, std::unique_ptr<Shader>> _shaders;
};
}
