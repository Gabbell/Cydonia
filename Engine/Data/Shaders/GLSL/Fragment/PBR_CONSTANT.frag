#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "LIGHTING.h"
#include "PBR.h"

layout( push_constant ) uniform PushMaterial
{
   layout(offset = 64)
   vec4 color;
   vec4 pbr; // X = metalness, Y = roughness, Z = AO, W = unused
};

layout( set = 1, binding = 1 ) uniform Lights { Light lights[MAX_LIGHTS]; };

// Interpolators
// =================================================================================================
layout( location = 0 ) in vec2 inTexCoord;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 worldPos;
layout( location = 3 ) in vec3 camPos;

layout( location = 0 ) out vec4 outColor;

// =================================================================================================
// This shader assumes that we are reading from sRGB formatted textures specified client-side and
// that we are outputting to an sRGB framebuffer
void main()
{
   const vec3 albedo = color.rgb;
   const vec3 normal = normalize( inNormal );

   const vec3 viewDir = normalize( camPos - worldPos );  // View direction

   vec3 totalRadiance = vec3( 0.0, 0.0, 0.0 );
   for( uint lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
   {
      if( lights[lightIdx].params.x > EPSILON )
      {
         const Light curLight = lights[lightIdx];

         float attenuation  = 1.0;
         vec3 lightDir      = vec3( 0.0, 0.0, 0.0 );
         vec3 lightRadiance = curLight.color.rgb;
         if( true /*pointLight*/ )
         {
            // Point Light
            const vec3 fragToLight = curLight.position.xyz - worldPos;
            const float distance   = length( fragToLight );
            attenuation            = 1.0;//1.0 / ( distance * distance );

            lightDir      = normalize( fragToLight );
            lightRadiance *= attenuation;
         }
         else
         {
            // Directional Light
            lightDir      = normalize( curLight.direction.xyz );
         }

         totalRadiance += ComputeLightCookTorranceBRDF(
             lightRadiance, lightDir, viewDir, albedo, normal, pbr.x, pbr.y, pbr.z );
      }
   }

   vec3 ambient = vec3(0.03) * albedo;
   vec3 finalColor = ambient + totalRadiance;

   // Tone map?

   outColor = vec4( finalColor, 1.0 );
}
