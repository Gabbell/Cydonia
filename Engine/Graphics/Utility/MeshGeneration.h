#pragma once

#include <cstdint>
#include <vector>

namespace CYD
{
class Vertex;

namespace MeshGeneration
{
// Returns a vector of vertices for a grid mesh centered at the origin (0, 0, 0). The actual length
// of the grid is always 1. Changing the width and the height only changes the resolution/detail of
// the grid. The primitive used for rendering should be triangle strips
void Grid(
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    uint32_t row,
    uint32_t columns );

void Icosphere(
    std::vector<Vertex>& vertices,
    std::vector<uint32_t>& indices,
    uint32_t subdivisions = 0,
    float divisionPerEdge = 2.0f );
}
}
