#include <Core/Graphics/Vulkan/ShaderStash.h>

#include <Core/Common/Assert.h>

#include <Core/Graphics/Vulkan/Device.h>
#include <Core/Graphics/Vulkan/Shader.h>

#include <filesystem>

// Hard-coded shader directories
static constexpr char RAW_SHADER_DIR[]      = "Resources/Shaders/Raw/";
static constexpr char COMPILED_SHADER_DIR[] = "Resources/Shaders/Compiled/";

cyd::ShaderStash::ShaderStash( const Device& device ) : _device( device )
{
   _initializeAllShaders();
}

void cyd::ShaderStash::_initializeAllShaders()
{
   for( const auto& entry : std::filesystem::directory_iterator( COMPILED_SHADER_DIR ) )
   {
      std::string shaderPath = entry.path().generic_string();

      _shaders.insert( { shaderPath, std::make_unique<Shader>( _device, shaderPath ) } );
   }
}

const cyd::Shader* cyd::ShaderStash::getShader( std::string shaderName )
{
   auto it = _shaders.find( COMPILED_SHADER_DIR + shaderName );
   if( it != _shaders.end() )
   {
      return it->second.get();
   }
   CYDASSERT( !"ShaderStash: Could not find shader in herder" );
   return nullptr;
}

cyd::ShaderStash::~ShaderStash() {}
