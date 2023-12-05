#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../LIGHTING.h"

layout( set = 0, binding = 4 ) uniform sampler2DShadow shadowMap;
layout( set = 1, binding = 1 ) uniform sampler2DArray normalMap;

layout( location = 0 ) in vec3 inUV;
layout( location = 1 ) in vec3 inWorldPos;
layout( location = 2 ) in vec3 inShadowCoord;

layout( location = 0 ) out vec4 outAlbedo;
layout( location = 1 ) out vec4 outNormal;
layout( location = 3 ) out vec4 outShadow;
layout( location = 4 ) out float outDepth;

// =================================================================================================
void main()
{
   // Determine terrain diffuse color
   const vec3 rockColor = vec3( 0.16, 0.07, 0.0 );
   const vec3 snowColor = vec3( 1.0, 1.0, 1.0 );

   vec3 unlitColor   = rockColor;
   const vec3 normal = ( normalize( texture( normalMap, inUV ).xyz ) * 2.0 ) - 1.0;
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
