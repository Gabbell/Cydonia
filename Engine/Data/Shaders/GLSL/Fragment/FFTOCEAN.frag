#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../ATMOS.h"
#include "../PBR.h"
#include "../SHADOW.h"
#include "../VIEW.h"
#include "../NOISE.h"

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 4 ) uniform LIGHTS { Light lights[MAX_LIGHTS]; };
layout( set = 0, binding = 5 ) uniform SHADOWMAPS { ShadowMap shadowMaps[MAX_SHADOW_MAPS]; };
layout( set = 0, binding = 6 ) uniform SHADOW_SAMPLER shadowMap;
layout( set = 0, binding = 7 ) uniform sampler2D envMap;

layout( set = 1, binding = 1 ) uniform sampler2D normalMap;
layout( set = 1, binding = 5 ) uniform sampler2D displacement;

layout( location = 0 ) in vec3 inUV;
layout( location = 1 ) in vec3 inVSPos;
layout( location = 2 ) in vec3 inWorldPos;

layout( location = 0 ) out vec4 outColor;

float diffuse( vec3 N, vec3 L ) { return max( dot( N, L ), 0.0 ); }

float specular( vec3 N, vec3 L, vec3 V, float exponent )
{
   const vec3 H = normalize( V + L );
   return pow( max( dot( N, H ), 0.0 ), exponent );
}

const vec3 SCATTER_COLOR = vec3( 0.0, 0.8, 1.0 );
const vec3 AMBIENT_COLOR = SCATTER_COLOR * 0.01;

const float SSS_MAX_HEIGHT    = 1.0;
const float OO_SSS_MAX_HEIGHT = 1.0 / SSS_MAX_HEIGHT;

// =================================================================================================
void main()
{
   const View mainView  = views[0];
   const Light curLight = lights[0];

   // Vectors
   // ============================================================================================
   const vec4 normalsFoam = texture( normalMap, inUV.xy );

   const vec4 D = texture( displacement, inUV.xy );
   const vec3 L = normalize( -curLight.direction.xyz );
   const vec3 N = normalize( normalsFoam.xyz );
   const vec3 V = normalize( inWorldPos - mainView.pos.xyz );  // viewToFrag

   // Factors
   // ============================================================================================
   const float fresnelSchlick = pow( clamp( 1.0 - dot( N, -V ), 0.0, 1.0 ), 5.0 );

   // Shadow Mapping
   // ============================================================================================
   const ShadowMap shadowMapParams = shadowMaps[0];

   float shadow = 1.0;
   if(shadowMapParams.enabled)
   {
      const vec4 shadowUV   = GetShadowUV(shadowMapParams, inWorldPos, inVSPos.z, N, L);
      shadow                = ShadowFactor( shadowMap, shadowMapParams, shadowUV );
   }

   // Environment Map
   // ============================================================================================
   vec3 reflectDir = normalize( reflect( V, N ) );

   // This fixes some black pixels that occur when reflectDir points downwards
   // and samples darkness
   reflectDir.y         = abs( reflectDir.y );
   const vec3 viewPosMM = getMMPosition( 6.36, mainView.pos.xyz );
   const float height   = viewPosMM.y;

   const float beta         = acos( sqrt( height * height - 6.36 * 6.36 ) / height );
   const float horizonAngle = PI - beta;

   const float altitudeAngle = horizonAngle - acos( dot( reflectDir, up ) );

   // Angle 0 has the sun in the positive Z direction (looking towards -Z)
   const float minusCosTheta = -dot( reflectDir, right );
   const float cosTheta      = dot( reflectDir, forward );
   const float azimuthAngle  = atan( minusCosTheta, cosTheta );

   // Making UVs, Y uses non-linear mapping formula from the paper
   const vec2 lookupUV = vec2(
       1.0 - abs( azimuthAngle / PI ),
       0.5 + 0.5 * sign( altitudeAngle ) * sqrt( abs( altitudeAngle ) / ( PI * 0.5 ) ) );

   vec3 envColor = texture( envMap, lookupUV ).rgb;

   // Color
   // ============================================================================================
   const vec3 lightRadiance = curLight.color.rgb;

   vec3 diffuseTerm  = vec3( 0.0 );
   vec3 specularTerm = vec3( 0.0 );

   const float sunFactor =
       clamp( smoothstep( 0.0, 1.0, dot( L, vec3( 0.0, 1.0, 0.0 ) ) ), 0.0, 1.0 );
   if( sunFactor > 0.0 && curLight.params.x > 0.0 )
   {
      // Subsurface Scattering
      const float ambientContribution = 0.1;
      const float waveHeightContribution =
          max( dot( V, L ), 0.0 ) * clamp( pow( D.y * OO_SSS_MAX_HEIGHT, 2.1 ), 0.0, 1.0 );
      const float normalContribution     = max( dot( -V, N ), 0.0 );
      const float lambertianContribution = max( dot( N, L ), 0.0 );

      const vec3 scatterColor = mix( SCATTER_COLOR * AMBIENT_COLOR, vec3( 1.0 ), normalsFoam.a );

      vec3 scatterTerm = ( ambientContribution + waveHeightContribution + normalContribution +
                           lambertianContribution ) *
                         scatterColor * lightRadiance;

      vec3 environmentTerm = envColor;

      float environmentContrib = fresnelSchlick;
      const float reflectDotLight = max(dot(reflectDir, L), 0.0);
      //environmentContrib *= mix(1.0 - reflectDotLight, 1.0, shadow);

      diffuseTerm = mix( scatterTerm, environmentTerm, environmentContrib );

      specularTerm += specular( N, L, -V, 2048 ) * lightRadiance * shadow;
   }

   outColor = vec4( diffuseTerm + specularTerm, 0.98 );
}