#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "LIGHTING.h"

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 inWorldPos;
layout( location = 3 ) in vec3 inCamPos;
layout( location = 4 ) in vec4 inShadowCoord;

layout( location = 0 ) out vec4 outColor;

// =================================================================================================
void main()
{
   vec3 bottomColor = vec3( 0.0, 1.0, 0.0 );
   vec3 topColor    = vec3( 1.0, 1.0, 1.0 );

   vec3 someColor = mix( bottomColor, topColor, inWorldPos.y / 10.0 );
   if( inWorldPos.y < 0.1 )
   {
      someColor = vec3( 0.05, 1.0, 1.0 );
   }

   vec3 normal = normalize( inNormal );

   float shadowFactor = ShadowPCF( inShadowCoord, normal, inWorldPos );

   Light curLight = lights[0];
   vec3 lightDir  = normalize( -curLight.direction.xyz );
   vec3 viewDir   = normalize( inCamPos - inWorldPos );
   vec3 lightContributions =
       ComputeLightBlinnPhong( curLight.color.rgb, lightDir, viewDir, normal, shadowFactor );

   outColor = vec4(someColor * lightContributions, 1.0 );
}
