#pragma once

#include <Common/Include.h>

#include <Graphics/GraphicsTypes.h>

namespace CYD
{
// ================================================================================================
// Layout Info
class VertexLayout
{
  public:
   VertexLayout() = default;
   COPIABLE( VertexLayout );
   ~VertexLayout() = default;

   enum class Attribute
   {
      UNKNOWN,
      POSITION,
      TEXCOORD,  // UV
      NORMAL,
      TANGENT,
      BITANGENT,
      COLOR
   };

   static constexpr uint32_t INVALID_LOCATION_OFFSET = 0xFFFFFFFF;

   struct AttributeInfo
   {
      Attribute type;
      PixelFormat vecFormat;
      uint32_t offset;
   };

   const std::vector<AttributeInfo>& getAttributes() const { return m_attributes; }
   bool isEmpty() const { return m_attributes.empty(); }
   uint32_t getStride() const { return m_stride; }
   uint32_t getLocationOffset( Attribute type ) const;

   // Location is implied by the order this is called.
   // Therefore, location is the index in m_attributes
   void addAttribute( Attribute type, PixelFormat vecFormat );

  private:
   std::vector<AttributeInfo> m_attributes;
   uint32_t m_stride = 0;
};
}