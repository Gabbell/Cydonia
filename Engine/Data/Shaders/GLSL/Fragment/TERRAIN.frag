#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "MATERIAL.h"

// View and environment
// =================================================================================================
layout( set = 0, binding = 1 ) uniform DirectionalLight
{
   mat4 viewMat;
   vec4 position;
   vec4 color;
   bool enabled;
}
lights;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 worldPos;
layout( location = 3 ) in vec3 camPos;

layout( location = 0 ) out vec4 outColor;

const float ambientStrength  = 0.1;
const float specularStrength = 0.5;

vec3 calcLightContribution( vec3 normal )
{
   vec3 lightColor = vec3( lights.color ) / 255.0;

   // Ambient
   vec3 ambient = ambientStrength * lightColor;

   // Diffuse
   //vec3 lightDir = normalize( vec3( lights.position ) - worldPos );
   vec3 lightDir = normalize( vec3( -0.5, 0.5, -0.5 ) );
   float diff    = max( dot( normal, lightDir ), 0.0 );
   vec3 diffuse  = diff * lightColor;

   // Specular
   vec3 specular = vec3( 0.0, 0.0, 0.0 );
   if( diff != 0.0 )  // Don't add a specular component when the diffuse dot product is 0
   {
      vec3 viewDir    = normalize( vec3( camPos ) - worldPos );
      vec3 reflectDir = reflect( -lightDir, normal );
      float spec      = pow( max( dot( normalize( viewDir ), reflectDir ), 0.0 ), 256 );
      vec3 specular   = specularStrength * spec * lightColor;
   }

   return ( ambient + diffuse + specular );
}

// =================================================================================================
void main()
{
   vec3 normal    = normalize( inNormal );

   vec3 bottomColor = vec3(0.0, 1.0, 0.0);
   vec3 topColor = vec3(1.0, 1.0, 1.0);

   vec3 someColor = mix(bottomColor, topColor, worldPos.y / 10.0);

   vec3 lightContributions = calcLightContribution( normal );

   outColor = vec4( someColor * lightContributions, 1.0 );
}
