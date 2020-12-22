#include <Graphics/ShaderConstants.h>

#include <Common/Assert.h>

#include <algorithm>

namespace CYD
{
const ShaderConstants::Entry* ShaderConstants::getEntry( const std::string& shaderName ) const
{
   const auto it = m_map.find( shaderName );
   if( it != m_map.end() )
   {
      return &it->second;
   }

   return nullptr;
}

template <class T>
void ShaderConstants::Entry::addConstant( uint32_t id, T value )
{
   const auto it = m_constantInfos.find( id );
   if( it != m_constantInfos.end() )
   {
      CYDASSERT( !"ShaderConstants: Cannot overwrite constants" );
      return;
   }

   m_constantInfos[id] = { static_cast<uint32_t>( m_data.size() ), sizeof( T ) };

   const size_t prevSize = m_data.size();
   m_data.resize( m_data.size() + sizeof( T ) );
   std::copy(
       static_cast<const char*>( static_cast<const void*>( &value ) ),
       static_cast<const char*>( static_cast<const void*>( &value ) ) + sizeof( T ),
       &m_data[prevSize] );
}

template void ShaderConstants::Entry::addConstant<int32_t>( uint32_t id, int32_t value );
template void ShaderConstants::Entry::addConstant<uint32_t>( uint32_t id, uint32_t value );
template void ShaderConstants::Entry::addConstant<float>( uint32_t id, float value );
template void ShaderConstants::Entry::addConstant<double>( uint32_t id, double value );

template <>
void ShaderConstants::Entry::addConstant<bool>( uint32_t id, bool value )
{
   addConstant( id, static_cast<uint32_t>( value ) );
}

template <class T>
void ShaderConstants::add( const std::string& shaderName, uint32_t id, T value )
{
   m_map[shaderName].addConstant( id, value );
}

template void
ShaderConstants::add<int32_t>( const std::string& shaderName, uint32_t id, int32_t value );
template void
ShaderConstants::add<uint32_t>( const std::string& shaderName, uint32_t id, uint32_t value );
template void
ShaderConstants::add<float>( const std::string& shaderName, uint32_t id, float value );
template void
ShaderConstants::add<double>( const std::string& shaderName, uint32_t id, double value );
template void ShaderConstants::add<bool>( const std::string& shaderName, uint32_t id, bool value );
}
