#pragma once

#include <cstdint>

// ================================================================================================
// Definition
// ================================================================================================
namespace cyd
{
enum class HandleType : uint32_t
{
   CMDLIST       = 0,
   VERTEXBUFFER  = 1,
   INDEXBUFFER   = 2,
   TEXTURE       = 3,
   UNIFORMBUFFER = 4
};

struct Handle
{
   Handle() : _index( 0 ), _counter( 0 ), _type( 0 ) {}

   Handle( uint32_t index, uint32_t counter, HandleType type )
       : _index( index ), _counter( counter ), _type( static_cast<uint32_t>( type ) )
   {
   }

   operator uint32_t() const { return _type << 27 | _counter << 12 | _index; }

   // The resource's index
   uint32_t _index : 12;

   // The number of times the resource was reused/updated
   uint32_t _counter : 15;

   // Used to determine the type of the data the handle is referring to
   uint32_t _type : 5;
};

using CmdListHandle       = Handle;
using VertexBufferHandle  = Handle;
using IndexBufferHandle   = Handle;
using TextureHandle       = Handle;
using UniformBufferHandle = Handle;

}  // namespace cyd
