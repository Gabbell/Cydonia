// LIGHTING.h
// Used for generic lighting in fragment shaders

#include "CONSTANTS.h"

// ================================================================================================
// Keep these defines and structs in sync with "LightShaderParams"
#define MAX_LIGHTS 3

struct Light
{
   vec4 position;   // XYZ = position, W = unused
   vec4 direction;  // XYZ = direction, W = unused
   vec4 color;      // RGB = color, A = unused
   vec4 params;     // X = enabled, Y = type, ZW = unused
};

// ================================================================================================

float ConstantAmbient()
{
   const float ambientTerm = 0.1;
   return ambientTerm;
}

float LambertianDiffuse( vec3 lightDir, vec3 normal )
{
   const float diffuseTerm = max( dot( normal, lightDir ), 0.0 );
   return diffuseTerm;
}

float ColorRampDiffuse( vec3 lightDir, vec3 normal )
{
   const float diffuseTerm = step( 0.5, max( dot( normal, lightDir ), 0.0 ) );
   return diffuseTerm;
}

float BlinnPhongSpecular( vec3 lightDir, vec3 viewDir, vec3 normal )
{
   const float specularStrength = 0.5;

   // Specular
   const vec3 halfDir       = normalize( lightDir + viewDir );
   const float spec         = pow( max( dot( normal, halfDir ), 0.0 ), 16.0 );
   const float specularTerm = specularStrength * spec;

   return specularTerm;
}

// Parallax Occlusion Mapping
// ================================================================================================
vec2 POMDown( sampler2D heightMap, vec2 inUV, vec3 TSviewDir, vec3 TSnormal )
{
   const float heightScale = 0.1;

   const uint minSamples = 4;
   const uint maxSamples = 32;

   float tMax = -length( TSviewDir.xy ) / TSviewDir.z;
   tMax *= heightScale;

   const vec2 offsetDir = normalize( TSviewDir.xy );
   const vec2 maxOffset = offsetDir * tMax;

   // We want to have more samples if the view direction and the normal are perpendicular
   const float lod       = 1.0 - dot( TSviewDir, TSnormal );
   const uint numSamples = int( mix( minSamples, maxSamples, lod ) );

   const float stepSize = 1.0 / numSamples;

   float currentRayHeight     = 1.0;
   vec2 currentUVOffset       = vec2( 0.0 );
   float currentSampledHeight = textureLod( heightMap, inUV + currentUVOffset, 0.0 ).r;

   vec2 prevUVOffset       = vec2( 0.0 );
   float prevSampledHeight = 0.0;

   uint currentSample = 1;
   while( currentSample < numSamples && currentSampledHeight < currentRayHeight )
   {
      // Post intersection not found, keep going
      currentRayHeight -= stepSize;
      prevUVOffset      = currentUVOffset;
      prevSampledHeight = currentSampledHeight;

      currentUVOffset += stepSize * maxOffset;

      currentSampledHeight = textureLod( heightMap, inUV + currentUVOffset, 0.0 ).r;
      currentSample++;
   }

   // Post intersection found
   float delta0    = currentSampledHeight - currentRayHeight;
   float delta1    = currentRayHeight + stepSize - prevSampledHeight;
   float ratio     = delta0 / ( delta0 + delta1 );
   currentUVOffset = ratio * prevUVOffset + ( 1.0 - ratio ) * currentUVOffset;

   return inUV + currentUVOffset;
}