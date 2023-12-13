#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../PBR.h"
#include "../COLOR.h"

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 1 ) uniform INVERSE_VIEWS { InverseView inverseViews[MAX_VIEWS]; };
layout( set = 0, binding = 2 ) uniform LIGHTS { Light lights[MAX_LIGHTS]; };

layout( set = 1, binding = 0 ) uniform sampler2D albedo;
layout( set = 1, binding = 1 ) uniform sampler2D normals;
layout( set = 1, binding = 2 ) uniform sampler2D pbr;
layout( set = 1, binding = 3 ) uniform sampler2D depth;
layout( set = 1, binding = 4 ) uniform sampler2D shadowMask;

layout( location = 0 ) in vec2 inUV;

layout( location = 0 ) out vec4 outColor;

void main()
{
   const vec3 albedo = texture( albedo, inUV ).rgb;
   const vec3 normal = texture( normals, inUV ).xyz;
   const vec4 pbr    = texture( pbr, inUV ).rgba;
   const vec4 shadow = texture( shadowMask, inUV );
   const float depth = texture( depth, inUV ).r;

   const float roughness = pbr.r;
   const float metalness = pbr.g;
   const float ao        = pbr.b;

   // Views
   const View mainView               = views[0];
   const InverseView inverseMainView = inverseViews[0];

   // Reconstruct world-space coordinates
   // Using view-space coordinates is not enough because it doesn't consider
   // the current view's rotation
   const vec4 ndc = vec4( inUV * 2.0 - 1.0, depth, 1.0 );
   vec4 worldPos  = inverseMainView.invViewMat * inverseMainView.invProjMat * ndc;
   worldPos       = worldPos / worldPos.w;

   const vec3 fragToView = normalize( mainView.pos.xyz - worldPos.xyz );  // View direction

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
            const vec3 fragToLight = curLight.position.xyz - worldPos.xyz;
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

         totalRadiance +=
             ComputeLightCookTorranceBRDF(
                 lightRadiance, lightDir, fragToView, albedo, normal, metalness, roughness, ao ) *
             shadow.r;
      }
   }

   // Add made-up ambient contribution
   totalRadiance += vec3( 0.005 ) * albedo * ao;

   outColor = vec4( totalRadiance, 1.0 );
}
