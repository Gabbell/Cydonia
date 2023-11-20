#pragma once

#include <cstdint>
#include <vector>

namespace CYD
{
class Vertex;
class VertexList;

namespace MeshGeneration
{
// Returns a vector of vertices for a grid mesh centered at the origin (0, 0, 0). The actual length
// of the grid is always 1. Changing the width and the height only changes the resolution/detail of
// the grid. The primitive used for rendering should be triangle strips
void TriangleGrid(
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    uint32_t row,
    uint32_t columns );

void PatchGrid(
    VertexList& vertexList,
    std::vector<uint32_t>& indices,
    uint32_t scale,
    uint32_t vertexResolution);

void Icosphere(
    VertexList& vertexList,
    std::vector<uint32_t>& indices,
    uint32_t subdivisions = 0,
    float divisionPerEdge = 2.0f );

void Octahedron( std::vector<Vertex>& vertices, std::vector<uint32_t>& indices );
}
}
