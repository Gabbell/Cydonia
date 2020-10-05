#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace CYD
{
struct Vertex;
struct TextureDescription;

namespace GraphicsIO
{
void LoadMesh(
    const std::string& path,
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices );

void* LoadImage( const TextureDescription& desc, const std::string& path );
void FreeImage( void* imageData );
}
}