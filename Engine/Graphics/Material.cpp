#include <Graphics/Material.h>

#include <Graphics/GRIS/RenderInterface.h>

namespace CYD
{
Material::~Material()
{
   GRIS::DestroyTexture( albedo );
   GRIS::DestroyTexture( normals );
   GRIS::DestroyTexture( disp );
   GRIS::DestroyTexture( metalness );
   GRIS::DestroyTexture( roughness );
   GRIS::DestroyTexture( ao );
}
}