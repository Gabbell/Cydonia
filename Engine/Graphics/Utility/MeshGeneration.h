#pragma once

#include <cstdint>
#include <vector>
#include <functional>

namespace CYD
{
class Vertex;
class VertexList;

namespace MeshGeneration
{
using Func = std::function<void()>;

// Returns a vector of vertices for a grid mesh centered at the origin (0, 0, 0). The actual length
// of the grid is "scale". Changing the row and column only changes the resolution/detail
// of the grid. The primitive used for rendering should be triangle strips
void TriangleGrid(
    VertexList& vertexList,
    std::vector<uint32_t>& indices,
    uint32_t scale,
    uint32_t row,
    uint32_t columns );

// Returns a patch used for tessellation centered at the origin (0, 0, 0). The actual size
// of the grid is "scale". The vertex resolution is how many vertices are present per sides
void PatchGrid(
    VertexList& vertexList,
    std::vector<uint32_t>& indices,
    uint32_t scale,
    uint32_t vertexResolution );

void Icosphere(
    VertexList& vertexList,
    std::vector<uint32_t>& indices,
    uint32_t subdivisions = 0,
    float divisionPerEdge = 2.0f );

void Octahedron( std::vector<Vertex>& vertices, std::vector<uint32_t>& indices );
}
}
