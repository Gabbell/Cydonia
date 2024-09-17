// Shadowing Header
// Contains shadow map sampling functions and generally useful constants for shadowing

#define MAX_SHADOW_MAPS 3

#define SHADOW_MAPPING_MAX_CASCADES 4  // Keep in sync with C counterpart in ShadowMapping namespace

struct ShadowCascade
{
   float farDepth;
   float radius;
   float texelSize;
   mat4 worldToLightMatrix;
   vec4 planes[6];
};

struct ShadowMap
{
   ShadowCascade cascades[SHADOW_MAPPING_MAX_CASCADES];
   bool enabled;
};

// ================================================================================================
// Shadow Mapping Implementations
#define BASIC_SHADOW_MAPPING 0x01
#define VARIANCE_SHADOW_MAPPING 0x02
#define EXP_SHADOW_MAPPING 0x03

#define SHADOW_MAPPING_IMP BASIC_SHADOW_MAPPING
#define SHADOW_MAPPING_CASCADED 1

#if SHADOW_MAPPING_IMP == BASIC_SHADOW_MAPPING

#define SHADOW_SAMPLER sampler2DArrayShadow  // PCF Sampler

#elif SHADOW_MAPPING_IMP == EXP_SHADOW_MAPPING

#define SHADOW_SAMPLER sampler2DArray

#endif

#define EXP_CONSTANT 80.0  // C
// ================================================================================================

vec4 ComputeShadowCoords( vec3 worldPos, mat4 lightProjMatrix, mat4 lightViewMatrix )
{
   vec4 shadowCoords = lightProjMatrix * lightViewMatrix * vec4( worldPos, 1.0 );
   shadowCoords /= shadowCoords.w;

   return shadowCoords;
}

vec4 ToUV( vec4 shadowCoords )
{
   return shadowCoords * vec4( 0.5, 0.5, 1.0, 1.0 ) + vec4( 0.5, 0.5, 0.0, 0.0 );
}

#if SHADOW_MAPPING_CASCADED

uint GetCascadeFromDepth( ShadowMap shadowMap, float vsDepth )
{
   uint cascade = 0;
   for( uint i = 0; i < SHADOW_MAPPING_MAX_CASCADES; ++i )
   {
      if( vsDepth < shadowMap.cascades[i].farDepth )
      {
         break;
      }

      cascade++;
   }

   return cascade;
}

#else

uint GetCascadeFromDepth( ShadowMap shadowMap, float depth ) { return 0; }

#endif

vec4 GetShadowUV(
    ShadowMap shadowMapParams,
    vec3 worldPos,
    float vsDepth,
    vec3 normal,
    vec3 lightDir )
{
#if SHADOW_MAPPING_IMP == BASIC_SHADOW_MAPPING
   const uint cascadeIndex     = GetCascadeFromDepth( shadowMapParams, vsDepth );
   const ShadowCascade cascade = shadowMapParams.cascades[cascadeIndex];

   const float shadowNormalOffset = 250000.0;  // Probably needs to be a smarter number

   const float cosLightAngle = dot( lightDir, normal );

   float normalOffsetScale = clamp( 1.0 - cosLightAngle, 0.0, 1.0 );
   normalOffsetScale *= shadowNormalOffset * cascade.texelSize;

   const vec4 shadowOffset = vec4( normal * normalOffsetScale, 0.0 );

   vec4 shadowCoord = cascade.worldToLightMatrix * vec4( worldPos, 1.0 );
   shadowCoord /= shadowCoord.w;

   vec4 shadowCoordBiased = cascade.worldToLightMatrix * ( vec4( worldPos, 1.0 ) + shadowOffset );
   shadowCoordBiased /= shadowCoordBiased.w;

   // Only biasing XY in UV space
   vec4 shadowUV = ToUV( vec4( shadowCoordBiased.xy, shadowCoord.z, 1.0 ) );
   shadowUV.z -= cascade.farDepth * 0.00000005;

   return vec4( shadowUV.xyz, cascadeIndex );

#else
   const uint cascadeIndex     = GetCascadeFromDepth( shadowMapParams, vsDepth );
   const ShadowCascade cascade = shadowMapParams.cascades[cascadeIndex];

   const vec3 shadowUV = ToUV( cascade.worldToLightMatrix * shadowCoord );

   return vec4( shadowUV.xyz, cascadeIndex );
#endif
}

#if SHADOW_MAPPING_IMP == BASIC_SHADOW_MAPPING
// ================================================================================================
// Basic Shadow Mapping
float ShadowFactor( SHADOW_SAMPLER shadowMap, ShadowMap shadowMapParams, vec4 shadowUV )
{
   if( shadowUV.z < 0.0 || shadowUV.z > 1.0 || shadowUV.x < 0.0 || shadowUV.x > 1.0 ||
       shadowUV.y < 0.0 || shadowUV.y > 1.0 )
   {
      return 1.0;
   }

   const ShadowCascade cascade = shadowMapParams.cascades[uint( shadowUV.w )];

   // We are offsetting less the greater the cascade is to have more consistent "bluriness"
   // across cascades
   const float cascadeOffsetRatio = shadowMapParams.cascades[0].radius / cascade.radius;

   const float texelOffset = cascade.texelSize * cascadeOffsetRatio;

   const uint KERNEL_SIDE   = 3;
   const uint KERNEL_SIZE   = ( KERNEL_SIDE * 2 ) + 1;
   const uint TOTAL_SAMPLES = KERNEL_SIZE * KERNEL_SIZE;

   const float HALF_KERNEL = KERNEL_SIZE / 2.0;
   const float WEIGHT      = 1.0 / TOTAL_SAMPLES;

   // PCF
   float shadow = 0.0;
   for( uint i = 0; i < TOTAL_SAMPLES; ++i )
   {
      const float x = ( i % KERNEL_SIZE ) - HALF_KERNEL;
      const float y = ( i / KERNEL_SIZE ) - HALF_KERNEL;

      const vec2 offset      = vec2( x, y ) * texelOffset;
      const vec4 curShadowUV = vec4( shadowUV.xy + offset, shadowUV.zw );
#if SHADOW_MAPPING_CASCADED
      shadow += WEIGHT * texture( shadowMap, curShadowUV.xywz );
#else
      shadow += WEIGHT * texture( shadowMap, curShadowUV.xyz );
#endif
   }

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
#elif SHADOW_MAPPING_IMP == EXP_SHADOW_MAPPING
// ================================================================================================
// Exponential Shadow Mapping
// View-space linear depth is expected from shadowCoords.z
float ShadowFactor( SHADOW_SAMPLER shadowMap, vec4 shadowUV, uint cascade )
{
   if( shadowUV.z < 0.0 || shadowUV.z > 1.0 || shadowUV.x < 0.0 || shadowUV.x > 1.0 ||
       shadowUV.y < 0.0 || shadowUV.y > 1.0 )
   {
      return 1.0;
   }

#if SHADOW_MAPPING_CASCADED
   const float occluder = texture( shadowMap, vec3( shadowUV.xy, cascade ) ).r;
#else
   const float occluder = texture( shadowMap, shadowUV.xy ).r;
#endif

   const float receiver = exp( -EXP_CONSTANT * shadowUV.z );

   const float shadow = occluder * receiver;

   // Threshold
   // const float epsilon = 0.02;
   // if ( shadow > 1.0 + epsilon )
   //{
   //   return 1.0;
   //}

   return clamp( shadow, 0.0, 1.0 );
}
#endif