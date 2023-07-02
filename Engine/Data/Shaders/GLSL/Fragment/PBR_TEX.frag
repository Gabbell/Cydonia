#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "LIGHTING.h"
#include "PBR.h"

// Material properties
// =================================================================================================
layout( set = 1, binding = 0 ) uniform sampler2D albedo;
layout( set = 1, binding = 1 ) uniform sampler2D normals;
layout( set = 1, binding = 2 ) uniform sampler2D metalness;
layout( set = 1, binding = 3 ) uniform sampler2D roughness;
layout( set = 1, binding = 4 ) uniform sampler2D ambientOcclusion;

// Interpolators
// =================================================================================================
layout( location = 0 ) in vec2 inTexCoord;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 worldPos;
layout( location = 3 ) in vec3 camPos;

layout( location = 0 ) out vec4 outColor;

// Helpers
// =================================================================================================
vec3 getNormalFromMap()
{
   vec3 tangentNormal = texture( normals, inTexCoord ).xyz * 2.0 - 1.0;

   vec3 Q1  = dFdx( worldPos );
   vec3 Q2  = dFdy( worldPos );
   vec2 st1 = dFdx( inTexCoord );
   vec2 st2 = dFdy( inTexCoord );

   vec3 N   = normalize( inNormal );
   vec3 T   = normalize( Q1 * st2.t - Q2 * st1.t );
   vec3 B   = -normalize( cross( N, T ) );
   mat3 TBN = mat3( T, B, N );

   return normalize( TBN * tangentNormal );
}

// =================================================================================================
// This shader assumes that we are reading from sRGB formatted textures specified client-side and
// that we are outputting to an sRGB framebuffer
void main()
{
   const vec3 albedo     = texture( albedo, inTexCoord.xy ).rgb;
   const vec3 normal     = getNormalFromMap();
   const float metalness = texture( metalness, inTexCoord.xy ).r;
   const float roughness = texture( roughness, inTexCoord.xy ).r;
   const float ao        = texture( ambientOcclusion, inTexCoord.xy ).r;

   const vec3 viewDir = normalize( camPos - worldPos );  // View direction

   vec3 totalRadiance = vec3( 0.0, 0.0, 0.0 );
   for( uint lightIdx = 0; lightIdx < MAX_LIGHTS; ++lightIdx )
   {
      if( lights[lightIdx].params.x > 0.0 )
      {
         const Light curLight = lights[lightIdx];

         float attenuation  = 1.0;
         vec3 lightDir      = vec3( 0.0, 0.0, 0.0 );
         vec3 lightRadiance = vec3( 0.0, 0.0, 0.0 );
         if( false /*pointLight*/ )
         {
            // Point Light
            const vec3 fragToLight = curLight.position.xyz - worldPos;
            const float distance   = length( fragToLight );
            attenuation            = 1.0 / ( distance * distance );

            lightDir      = normalize( fragToLight );
            lightRadiance = curLight.color.rgb * attenuation;
         }
         else
         {
            // Directional Light
            lightDir      = normalize( curLight.direction.xyz );
            lightRadiance = curLight.color.rgb;
         }

         totalRadiance += ComputeLightCookTorranceBRDF(
             lightRadiance, lightDir, viewDir, albedo, normal, metalness, roughness, ao );
      }
   }

   outColor = vec4( totalRadiance, 1.0 );
}
