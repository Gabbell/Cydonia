#pragma once

#include <Graphics/Handles/ResourceHandle.h>

namespace CYD
{
struct Mesh
{
   Mesh() = default;
   ~Mesh();

   VertexBufferHandle vertexBuffer;
   IndexBufferHandle indexBuffer;
   uint32_t vertexCount = 0;
   uint32_t indexCount  = 0;
};
}