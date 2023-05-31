#pragma once

#include <Graphics/Scene/Mesh.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
Mesh::~Mesh()
{
   GRIS::DestroyBuffer( vertexBuffer );
   GRIS::DestroyBuffer( indexBuffer );
}
}