#pragma once

#include <Common/Include.h>

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace CYD
{
class ShaderConstants final
{
  public:
   ShaderConstants() = default;
   COPIABLE( ShaderConstants );
   ~ShaderConstants() = default;

   struct Info
   {
      // Each constant has a size and an offset in the data buffer
      uint32_t offset;
      size_t size;
   };

   // Shader constant info per constant ID
   using InfoMap = std::unordered_map<uint32_t, Info>;

   struct Entry
   {
      const void* getData() const { return m_data.data(); }
      size_t getDataSize() const { return sizeof( m_data[0] ) * m_data.size(); }

      const InfoMap& getConstantInfos() const { return m_constantInfos; }

      template <typename T>
      void addConstant( uint32_t id, T value );

     private:
      InfoMap m_constantInfos;

      // Byte data buffer containing all constant values
      std::vector<unsigned char> m_data;
   };

   // Will return nullptr if there is no entry for this particular shader
   const Entry* getEntry( const std::string& shaderName ) const;

   template <typename T>
   void add( const std::string& shaderName, uint32_t id, T value );

  private:
   // Map containing per shader shader constants
   std::unordered_map<std::string, Entry> m_map;
};
}
