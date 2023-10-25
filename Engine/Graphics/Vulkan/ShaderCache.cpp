#include <Graphics/Vulkan/ShaderCache.h>

#include <Common/Assert.h>

#include <Graphics/Vulkan/Shader.h>

#include <filesystem>

// Hard-coded shader directories
static constexpr char SPIRV_SHADER_DIR[] = "../Shaders/";

namespace vk
{
ShaderCache::ShaderCache( const Device& device ) : m_device( device ) { _initializeAllShaders(); }
ShaderCache::~ShaderCache() = default;

void ShaderCache::_initializeAllShaders()
{
   CYD_ASSERT(
       std::filesystem::exists( SPIRV_SHADER_DIR ) &&
       "ShaderCache: Could not find compiled shader directory" );

   auto directory = std::filesystem::directory_iterator( SPIRV_SHADER_DIR );

   for( const auto& entry : directory )
   {
      std::string shaderPath = entry.path().generic_string();

      m_shaders.insert( { shaderPath, std::make_unique<Shader>( m_device, shaderPath ) } );
   }
}

const Shader* ShaderCache::getShader( const std::string& shaderName )
{
   auto it = m_shaders.find( SPIRV_SHADER_DIR + shaderName + ".spv" );
   if( it != m_shaders.end() )
   {
      return it->second.get();
   }
   CYD_ASSERT( !"ShaderCache: Could not find shader in cache" );
   return nullptr;
}

void ShaderCache::reset()
{
   m_shaders.clear();
   _initializeAllShaders();
}
}