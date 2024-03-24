#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../PBR.h"

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };
layout( set = 0, binding = 1 ) uniform Lights { Light lights[MAX_LIGHTS]; };

// Material properties
// =================================================================================================
layout( set = 1, binding = 0 ) uniform sampler2D albedo;
layout( set = 1, binding = 1 ) uniform sampler2D normals;
layout( set = 1, binding = 2 ) uniform sampler2D metalness;
layout( set = 1, binding = 3 ) uniform sampler2D roughness;
layout( set = 1, binding = 4 ) uniform sampler2D ambientOcclusion;

// Interpolators
// =================================================================================================
layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inWorldPos;
layout( location = 2 ) in vec3 inShadowCoord;
layout( location = 3 ) in mat3 inTBN;

layout( location = 0 ) out vec4 outColor;

// =================================================================================================
void main()
{
   const View mainView = views[0];

   const vec3 albedo     = texture( albedo, inUV.xy ).rgb;
   const vec3 TSNormal   = texture( normals, inUV ).xyz * 2.0 - 1.0;  // Tangent space
   const float metalness = texture( metalness, inUV.xy ).r;
   const float roughness = texture( roughness, inUV.xy ).r;
   const float ao        = texture( ambientOcclusion, inUV.xy ).r;

   const vec3 fragToView = normalize( mainView.pos.xyz - inWorldPos );  // View direction
   const vec3 normal     = normalize( inTBN * normalize( TSNormal ) );

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
            const vec3 fragToLight = curLight.position.xyz - inWorldPos;
            const float distance   = length( fragToLight );
            attenuation            = 1.0 / ( distance * distance );

            lightDir      = normalize( fragToLight );
            lightRadiance = curLight.color.rgb * attenuation;
         }
         else
         {
            // Directional Light
            lightDir      = normalize( -curLight.direction.xyz );
            lightRadiance = curLight.color.rgb;
         }

         totalRadiance += ComputeLightCookTorranceBRDF(
             lightRadiance, lightDir, fragToView, albedo, normal, metalness, roughness, ao );
      }
   }

   totalRadiance += vec3( 0.03 ) * albedo * ao;

   outColor = vec4( totalRadiance, 1.0 );
}
