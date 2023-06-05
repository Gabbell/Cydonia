#include <Graphics/Vulkan/ShaderCache.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan/Shader.h>

#include <filesystem>

// Hard-coded shader directories
static constexpr char SPIRV_SHADER_DIR[] = "Shaders/";

namespace vk
{
ShaderCache::ShaderCache( const Device& device ) : _device( device ) { _initializeAllShaders(); }
ShaderCache::~ShaderCache() = default;

void ShaderCache::_initializeAllShaders()
{
   CYDASSERT(
       std::filesystem::exists( SPIRV_SHADER_DIR ) &&
       "ShaderCache: Could not find compiled shader directory" );

   auto directory = std::filesystem::directory_iterator( SPIRV_SHADER_DIR );

   for( const auto& entry : directory )
   {
      std::string shaderPath = entry.path().generic_string();

      _shaders.insert( { shaderPath, std::make_unique<Shader>( _device, shaderPath ) } );
   }
}

const Shader* ShaderCache::getShader( const std::string& shaderName )
{
   auto it = _shaders.find( SPIRV_SHADER_DIR + shaderName + ".spv" );
   if( it != _shaders.end() )
   {
      return it->second.get();
   }
   CYDASSERT( !"ShaderCache: Could not find shader in cache" );
   return nullptr;
}
}