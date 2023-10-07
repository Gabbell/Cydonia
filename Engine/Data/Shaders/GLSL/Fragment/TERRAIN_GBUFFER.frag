#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "LIGHTING.h"

layout( set = 1, binding = 1 ) uniform sampler2DShadow shadowMap;
layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inWorldPos;
layout( location = 2 ) in vec3 inShadowCoord;

layout( location = 0 ) out vec4 outAlbedo;
layout( location = 1 ) out vec4 outNormal;
layout( location = 2 ) out vec4 outShadow;
layout( location = 3 ) out float outDepth;

// Bump mapping based on heightmap
vec3 getNormals( vec2 uv )
{
   const vec2 texelSize = 1.0 / textureSize( heightMap, 0 );
   const ivec3 off      = ivec3( -1, 0, 1 );
   const vec3 size      = vec3( 2.0 * texelSize.x, 2.0 * texelSize.y, 0.0 );

   // Uses 4 samples to create one normal, effectively "blurring" the normals
   // I don't think these are scaled properly, noon looks weird
   float hL = textureOffset( heightMap, uv, off.xy ).r;
   float hR = textureOffset( heightMap, uv, off.zy ).r;
   float hU = textureOffset( heightMap, uv, off.yx ).r;
   float hD = textureOffset( heightMap, uv, off.yz ).r;

   vec3 va = vec3( size.x, hR - hL, size.z );
   vec3 vb = vec3( size.z, hU - hD, -size.y );

   /*
   // 3 samples variant
   float hO = texture(heightMap, uv).r;
   float hR = textureOffset( heightMap, uv, off.zy ).r;
   float hU = textureOffset( heightMap, uv, off.yx ).r;

   vec3 va = (vec3(size.x, hR - hO, size.y));
   vec3 vb = (vec3(size.y, hU - hO, -size.x));
   */

   vec3 up = vec3( 0.0, 1.0, 0.0 );
   return normalize( up + normalize( cross( va, vb ) ) );
}

// =================================================================================================
void main()
{
   // Determine terrain diffuse color
   vec3 bottomColor = vec3( 0.0, 1.0, 0.0 );
   vec3 topColor    = vec3( 0.0, 1.0, 0.0 );

   vec3 unlitColor = mix( bottomColor, topColor, inWorldPos.y / 10.0 );
   if( inWorldPos.y < 0.1 )
   {
      unlitColor = vec3( 0.05, 1.0, 1.0 );
   }

   const vec3 normal = getNormals( inUV );

   outAlbedo = vec4( unlitColor, 1.0 );
   outShadow = vec4( ShadowPCF( shadowMap, inShadowCoord ), 0.0, 0.0, 1.0 );
   outNormal = vec4( normal, 1.0 );
   outDepth  = gl_FragCoord.z;
}
