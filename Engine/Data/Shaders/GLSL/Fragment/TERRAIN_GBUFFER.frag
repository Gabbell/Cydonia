#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../LIGHTING.h"

layout( set = 0, binding = 4 ) uniform sampler2DShadow shadowMap;
layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inWorldPos;
layout( location = 2 ) in vec3 inShadowCoord;

layout( location = 0 ) out vec4 outAlbedo;
layout( location = 1 ) out vec4 outNormal;
layout( location = 3 ) out vec4 outShadow;
layout( location = 4 ) out float outDepth;

// Bump mapping based on heightmap
// This should be done only once and baked
vec3 getNormals( vec2 uv )
{
   const ivec3 off = ivec3( -1, 0, 1 );

   const float hO = texture( heightMap, uv ).r*500.0;
   const float hL = textureOffset( heightMap, uv, off.xy ).r*500.0;
   const float hR = textureOffset( heightMap, uv, off.zy ).r*500.0;
   const float hT = textureOffset( heightMap, uv, off.yx ).r*500.0;
   const float hB = textureOffset( heightMap, uv, off.yz ).r*500.0;

   const vec3 left   = vec3( -1.0, hL, 0.0 ) - hO;
   const vec3 right  = vec3( 1.0, hR, 0.0 ) - hO;
   const vec3 top    = vec3( 0.0, hT, -1.0 ) - hO;
   const vec3 bottom = vec3( 0.0, hB, 1.0 ) - hO;

   const vec3 topRight    = cross( right, top );
   const vec3 topLeft     = cross( top, left );
   const vec3 bottomLeft  = cross( left, bottom );
   const vec3 bottomRight = cross( bottom, right );

   /*
   // 3 samples variant
   float hO = texture(heightMap, uv).r;
   float hR = textureOffset( heightMap, uv, off.zy ).r;
   float hU = textureOffset( heightMap, uv, off.yx ).r;

   vec3 va = (vec3(size.x, hR - hO, size.y));
   vec3 vb = (vec3(size.y, hU - hO, -size.x));
   */

   return normalize( topRight + topLeft + bottomLeft + bottomRight );
}

// =================================================================================================
void main()
{
   // Determine terrain diffuse color
   const vec3 rockColor = vec3( 0.16, 0.07, 0.0 );
   const vec3 snowColor = vec3( 1.0, 1.0, 1.0 );

   vec3 unlitColor   = rockColor;
   const vec3 normal = getNormals( inUV );
   const vec3 up     = vec3( 0.0, 1.0, 0.0 );
   if( dot( up, normal ) > 0.2 )
   {
      unlitColor = snowColor;
   }

   outAlbedo = vec4( unlitColor, 1.0 );
   outShadow = vec4( ShadowPCF( shadowMap, inShadowCoord ), 0.0, 0.0, 1.0 );
   outNormal = vec4( normal, 1.0 );
   outDepth  = gl_FragCoord.z;
}
