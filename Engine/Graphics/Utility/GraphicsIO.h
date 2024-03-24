#pragma once

#include <Graphics/GraphicsTypes.h>

#include <cstdint>
#include <string>
#include <vector>

namespace CYD
{
class VertexList;

namespace GraphicsIO
{
void LoadMesh( const std::string& path, VertexList& vertices, std::vector<uint32_t>& indices );

void* LoadImage(
    const std::string& path,
    PixelFormat format,
    uint32_t& width,
    uint32_t& height,
    uint32_t& size );
void FreeImage( void* imageData );
}
}