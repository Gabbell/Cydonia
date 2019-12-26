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
   Handle() : index( 0 ), counter( 0 ), type( 0 ) {}

   Handle( uint32_t index, uint32_t counter, HandleType type )
       : index( index ), counter( counter ), type( static_cast<uint32_t>( type ) )
   {
   }

   operator uint32_t() const { return type << 27 | counter << 12 | index; }

   // The resource's index
   uint32_t index : 12;

   // The number of times the resource was reused/updated
   uint32_t counter : 15;

   // Used to determine the type of the data the handle is referring to
   uint32_t type : 5;
};

using CmdListHandle       = Handle;
using VertexBufferHandle  = Handle;
using IndexBufferHandle   = Handle;
using TextureHandle       = Handle;
using UniformBufferHandle = Handle;
}
