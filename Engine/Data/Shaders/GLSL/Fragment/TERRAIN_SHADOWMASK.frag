#version 450

#include "../VIEW.h"
#include "../LIGHTING.h"
#include "../SHADOW.h"
#include "../NOISE.h"

layout( push_constant ) uniform PushConstant { mat4 invModel; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 1 ) uniform INVERSE_VIEWS { InverseView inverseViews[MAX_VIEWS]; };
layout( set = 0, binding = 2 ) uniform LIGHTS { Light lights[MAX_LIGHTS]; };
layout( set = 0, binding = 3 ) uniform SHADOWMAPS { ShadowMap shadowMaps[MAX_SHADOW_MAPS]; };

layout( set = 0, binding = 4 ) uniform SHADOW_SAMPLER shadowMap;
layout( set = 0, binding = 5 ) uniform sampler2D depth;
layout( set = 0, binding = 6 ) uniform sampler2DArray heightMap;

layout( location = 0 ) in vec2 inUV;

layout( location = 0 ) out vec2 outShadow;

const uint MAX_VOLUME_SHADOW_STEPS = 16;

/*
float GetTerrainHeight( vec3 p )
{
   // TODO Need to be dynamic
   // All this is in XZ space
   const vec2 tileSize    = vec2( 128.0, 128.0 );
   const vec2 gridSize    = vec2( 8.0, 8.0 );                // 64 tiles
   const vec2 groupOrigin = -tileSize * ( gridSize / 2.0 );  // Based on camera pos

   const vec4 msPos   = invModel * vec4( p, 1.0 );              // Model-space position
   const vec2 gridPos = ( msPos.xz - groupOrigin ) / tileSize;  // Grid-space position
   if( gridPos.x > 8.0 || gridPos.y > 8.0 || gridPos.x < 0.0 || gridPos.y < 0.0 ) return 0.0;

   const float instanceIndex = floor( gridPos.x ) + floor( gridPos.y ) * gridSize.y;

   const vec3 uv             = vec3( fract( gridPos ), instanceIndex );
   const float terrainHeight = texture( heightMap, uv ).r;

   return ( 1.0 / invModel[1][1] ) * terrainHeight - 80.0;
}

float ShadowRayMarch( vec3 ro, vec3 rd, float tOffset, float tMax, uint maxSteps )
{
   float shadow = 1.0;

   const float stepSize = tMax / maxSteps;

   float t = tOffset;
   while( t < tMax )
   {
      const vec3 point = ro + rd * t;

      const float terrainHeight = GetTerrainHeight( point );

      t += stepSize;
      if( point.y < terrainHeight )
      {
         // Intersection
         shadow = 0.0;
         break;
      }
   }

   return shadow;
}
*/

void main()
{
   const InverseView inverseMainView = inverseViews[0];
   const Light sunLight              = lights[0];
   const View sunView                = views[1];

   // Reconstruct world-space coordinates
   const float depth = texture( depth, inUV ).r;  // From 0 to 1
   const vec4 ndc    = vec4( inUV * 2.0 - 1.0, depth, 1.0 );
   vec4 vsPos        = inverseMainView.invProjMat * ndc;
   vsPos /= vsPos.w;

   // Shadow Mapping
   // =============================================================================================
   const ShadowMap shadowMapParams = shadowMaps[0];

   // In view space
   const vec3 rayDestination = vsPos.xyz;
   const float tMax          = vsPos.z;
   const vec3 rayDirection   = normalize( rayDestination );

   const float startJitter = 200.0;
   const vec3 startPoint = rayDirection * startJitter * ( hash2( inUV ).x * 2.0 - 1.0 );

   const float stepSize = tMax / MAX_VOLUME_SHADOW_STEPS;

   // Moving towards the depth-determined intersection
   float transmittance = 1.0;
   float lightEnergy = 0.0;
   float t = 0.0;
   while( t < tMax )
   {
      const vec3 point = startPoint + rayDirection * t;
      const float sampleTransmittance = exp( ( -stepSize / 1e6 ) );

      // Only looking up in last cascade as an optimization
      const uint cascadeIndex = SHADOW_MAPPING_MAX_CASCADES - 1;
      //const uint cascadeIndex     = GetCascadeFromDepth( shadowMapParams, point.z );
      const ShadowCascade cascade = shadowMapParams.cascades[cascadeIndex];

      const vec4 worldPos = inverseMainView.invViewMat * vec4( point, 1.0 );
      
      vec4 shadowUV = ToUV( cascade.worldToLightMatrix * worldPos );
      shadowUV.w = cascadeIndex;

      const float occlusion = ShadowFactor( shadowMap, shadowMapParams, shadowUV );

      lightEnergy += transmittance * occlusion;

      transmittance *= sampleTransmittance;

      t += stepSize;
   }

   lightEnergy = clamp( lightEnergy / MAX_VOLUME_SHADOW_STEPS, 0.0, 1.0 );

   // Set the output color
   outShadow = vec2( lightEnergy, 0.0 );
}