#pragma once

#include <Graphics/GraphicsTypes.h>

#include <cstdint>
#include <string>
#include <vector>

namespace CYD
{
class Vertex;

namespace GraphicsIO
{
void LoadMesh(
    const std::string& path,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices );

void* LoadImage( const std::string& path, PixelFormat format, int& width, int& height, int& size );
void FreeImage( void* imageData );
}
}