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

vec3 GammaToLinear( vec3 color )
{
   return vec3( pow( color.r, 2.2 ), pow( color.g, 2.2 ), pow( color.b, 2.2 ) );
}

vec3 LinearToGamma( vec3 color )
{
   return vec3( pow( color.r, 1.0 / 2.2 ), pow( color.g, 1.0 / 2.2 ), pow( color.b, 1.0 / 2.2 ) );
}

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

float ShadowPCF( sampler2DShadow shadowMap, vec3 shadowCoords )
{
   // ShadowCoords are NDC coordinates of the current fragment from the point of view of the light
   if( shadowCoords.z <= -1.0 || shadowCoords.z >= 1.0 || shadowCoords.x < 0.0 ||
       shadowCoords.x > 1.0 || shadowCoords.y < 0.0 || shadowCoords.y > 1.0 )
   {
      return 1.0;
   }

   // Calculate bias (based on depth map resolution and slope)
   // const vec3 lightDir = normalize( lightPos - worldPos );
   // float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);

   // PCF
   const vec2 texelSize = 1.0 / textureSize( shadowMap, 0 );
   const int range      = 1;  // 9 Samples
   int count            = 0;
   float shadow         = 0.0;
   for( int x = -range; x <= range; ++x )
   {
      for( int y = -range; y <= range; ++y )
      {
         const vec2 offset          = vec2( x, y ) * texelSize;
         const vec3 curShadowCoords = vec3( shadowCoords.xy + offset, shadowCoords.z );
         shadow += texture( shadowMap, curShadowCoords );
         count++;
      }
   }

   shadow /= count;

   // Far fade
   /*
   const float fade      = 0.01;  // Last 1% of shadow result are linearly faded
   const float startFade = 1.0 - fade;

   const vec2 ndcShadowCoords = shadowCoords.xy * 2.0 - 1.0;  // XY from -1.0 to 1.0 centered
   const float maxCoord =
       clamp( max( abs( ndcShadowCoords.x ), abs( ndcShadowCoords.y ) ), 0.0, 1.0 );

   float edgeFade = 1.0;
   if( maxCoord > startFade && maxCoord <= 1.0 )
   {
      const float t = ( maxCoord - startFade ) / fade;
      edgeFade   a   = mix( 1.0, 0.0, t );
   }
   */

   return shadow;
}

// Parallax Occlusion Mapping
// ================================================================================================
vec2 POMDown( sampler2D heightMap, vec2 inUV, vec3 TSviewDir, vec3 TSnormal )
{
   const float heightScale = 0.1;

   const int minSamples = 4;
   const int maxSamples = 32;

   float tMax = -length( TSviewDir.xy ) / TSviewDir.z;
   tMax *= heightScale;

   const vec2 offsetDir = normalize( TSviewDir.xy );
   const vec2 maxOffset = offsetDir * tMax;

   // We want to have more samples if the view direction and the normal are perpendicular
   const float lod      = 1.0 - dot( TSviewDir, TSnormal );
   const int numSamples = int( mix( minSamples, maxSamples, lod ) );

   const float stepSize = 1.0 / numSamples;

   float currentRayHeight     = 1.0;
   vec2 currentUVOffset       = vec2( 0.0 );
   float currentSampledHeight = textureLod( heightMap, inUV + currentUVOffset, 0.0 ).r;

   vec2 prevUVOffset       = vec2( 0.0 );
   float prevSampledHeight = 0.0;

   int currentSample = 1;
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