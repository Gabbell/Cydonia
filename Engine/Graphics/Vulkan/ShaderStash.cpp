#include <Graphics/Vulkan/ShaderStash.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan/Shader.h>

#include <filesystem>

// Hard-coded shader directories
static constexpr char SPIRV_SHADER_DIR[] = "Data/Shaders/SPIR-V/";

namespace vk
{
ShaderStash::ShaderStash( const Device& device ) : _device( device ) { _initializeAllShaders(); }
ShaderStash::~ShaderStash() = default;

void ShaderStash::_initializeAllShaders()
{
   auto directory = std::filesystem::directory_iterator( SPIRV_SHADER_DIR );
   CYDASSERT( directory->exists() && "ShaderStash: Could not find compiled shader directory" );

   for( const auto& entry : directory )
   {
      std::string shaderPath = entry.path().generic_string();

      _shaders.insert( {shaderPath, std::make_unique<Shader>( _device, shaderPath )} );
   }
}

const Shader* ShaderStash::getShader( const std::string& shaderName )
{
   auto it = _shaders.find( SPIRV_SHADER_DIR + shaderName + ".spv" );
   if( it != _shaders.end() )
   {
      return it->second.get();
   }
   CYDASSERT( !"ShaderStash: Could not find shader in herder" );
   return nullptr;
}
}