#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../LIGHTING.h"
#include "../SHADOW.h"
#include "../NOISE.h"

layout( set = 0, binding = 2 ) uniform LIGHTS { Light lights[MAX_LIGHTS]; };
layout( set = 0, binding = 3 ) uniform SHADOWMAPS { ShadowMap shadowMaps[MAX_SHADOW_MAPS]; };
layout( set = 0, binding = 4 ) uniform SHADOW_SAMPLER shadowMap;

layout( set = 1, binding = 1 ) uniform sampler2DArray normalMap;

layout( location = 0 ) in vec3 inUV;
layout( location = 1 ) in float inVSDepth;
layout( location = 2 ) in vec3 inWorldPos;
layout( location = 3 ) in vec3 inNormal;

layout( location = 0 ) out vec4 outAlbedo;
layout( location = 1 ) out vec4 outNormal;
layout( location = 2 ) out vec4 outPBR;
layout( location = 3 ) out vec4 outShadow;
layout( location = 4 ) out float outDepth;

// =================================================================================================
const float SAND_START  = -100.0;
const float GRASS_START = 100.0;
const float ROCK_START  = 2000.0;

const float GRASS_BLEND = 10.0;
const float ROCK_BLEND  = 200.0;

const float SAND_END  = GRASS_START + GRASS_BLEND;
const float GRASS_END = ROCK_START + ROCK_BLEND;

void main()
{
   const vec3 normal = normalize( texture( normalMap, inUV ).xyz * 2.0 - 1.0 );

   // Determine terrain diffuse color
   const vec3 sandColor  = vec3( 0.761, 0.698, 0.502 );
   const vec3 grassColor = vec3( 0.149, 0.545, 0.027 );
   const vec3 rockColor  = vec3( 0.16, 0.07, 0.0 );
   const vec3 snowColor  = vec3( 1.0, 1.0, 1.0 );

   float grassPresence = clamp( ( inWorldPos.y - GRASS_START ) / GRASS_BLEND, 0.0, 1.0 );
   float rockPresence  = clamp( ( inWorldPos.y - ROCK_START ) / ROCK_BLEND, 0.0, 1.0 );

   float roughness = mix( 0.6, 1.0, rockPresence );

   vec3 unlitColor = mix( sandColor, grassColor, grassPresence );
   unlitColor      = mix( unlitColor, rockColor, rockPresence );
   const vec3 up   = vec3( 0.0, 1.0, 0.0 );
   if( dot( up, normal ) > mix( 1.5, 0.75, rockPresence ) )
   {
      // There is snow
      roughness  = 0.4;
      unlitColor = snowColor;
   }

   // DEBUG One color per instance
   // unlitColor = vec3(hash1(inUV.z), hash1(inUV.z + 1), hash1(inUV.z + 2));

   // Shadow Mapping
   // =============================================================================================
   const Light sunLight = lights[0];
   const vec3 sunDir    = -sunLight.direction.xyz;

   const ShadowMap shadowMapParams = shadowMaps[0];

   float shadow = 1.0;
   if( shadowMapParams.enabled )
   {
      const vec4 shadowUV = GetShadowUV( shadowMapParams, inWorldPos, inVSDepth, normal, sunDir );
      shadow              = ShadowFactor( shadowMap, shadowMapParams, shadowUV );
   }

   outAlbedo = vec4( unlitColor, 1.0 );
   outNormal = vec4( normal, 1.0 );
   outPBR    = vec4( roughness, 0.0, 1.0, 1.0 );
   outShadow = vec4( shadow, 0.0, 0.0, 1.0 );
   outDepth  = gl_FragCoord.z;
}
