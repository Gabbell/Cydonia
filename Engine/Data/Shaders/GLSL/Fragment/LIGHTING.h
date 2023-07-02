// LIGHTING.h
// Used for generic lighting in fragment shaders

// ================================================================================================
// Keep these defines and structs in sync with "LightShaderParams"
#define MAX_LIGHTS 3
#define EPSILON 0.0001

struct Light
{
   vec4 position;   // XYZ = position, W = unused
   vec4 direction;  // XYZ = direction, W = unused
   vec4 color;      // RGB = color, A = unused
   vec4 params;     // X = enabled, Y = type, ZW = unused
};

layout( set = 1, binding = 1 ) uniform Lights { Light lights[MAX_LIGHTS]; };
layout( set = 1, binding = 2 ) uniform sampler2D shadowMap;

// ================================================================================================

vec3 GammaToLinear( vec3 color )
{
   return vec3( pow( color.r, 2.2 ), pow( color.g, 2.2 ), pow( color.b, 2.2 ) );
}

vec3 LinearToGamma( vec3 color )
{
   return vec3( pow( color.r, 1.0 / 2.2 ), pow( color.g, 1.0 / 2.2 ), pow( color.b, 1.0 / 2.2 ) );
}

vec3 ComputeLightBlinnPhong(
    vec3 lightRadiance,
    vec3 lightDir,
    vec3 viewDir,
    vec3 normal,
    float shadowFactor )
{
   const float ambientStrength  = 0.1;
   const float specularStrength = 0.5;

   // Ambient
   vec3 ambientTerm = ambientStrength * lightRadiance;

   // Lambertian Diffuse
   float diff       = clamp( dot( normal, lightDir ), 0.0, 1.0 );
   vec3 diffuseTerm = diff * lightRadiance;

   // Specular
   vec3 specularTerm = vec3( 0.0, 0.0, 0.0 );
   if( diff != 0.0 )  // Don't add a specular component when the diffuse dot product is 0
   {
      vec3 halfDir = normalize( lightDir + viewDir );
      float spec   = pow( clamp( dot( normal, halfDir ), 0.0, 1.0 ), 16.0 );
      specularTerm = specularStrength * spec * lightRadiance;
   }

   return ( ambientTerm + shadowFactor * ( diffuseTerm + specularTerm ) );
}

float CalculateShadow( vec4 shadowCoords, float bias, vec2 offset )
{
   // By default, we want no shadows
   // If sampling outside of expected range, return no shadow
   float shadow = 1.0;
   if( shadowCoords.z > -1.0 && shadowCoords.z < 1.0 )
   {
      float dist = texture( shadowMap, shadowCoords.xy + offset ).r;
      if( shadowCoords.w > 0.0 && dist < ( shadowCoords.z - bias ) )
      {
         shadow = 0.0;
      }
   }

   return shadow;
}

float ShadowPCF( vec4 shadowCoords, vec3 normal, vec3 worldPos )
{
   // ShadowCoords are NDC coordinates of the current fragment from the point of view of the light
   shadowCoords = shadowCoords / shadowCoords.w;

   // Calculate bias (based on depth map resolution and slope)
   vec3 lightDir = normalize( lights[0].position.xyz - worldPos );
   float bias    = max( 0.05 * ( 1.0 - dot( normal, lightDir ) ), 0.005 );

   // PCF
   vec2 texelSize = 1.0 / textureSize( shadowMap, 0 );
   float shadow   = 0.0;
   int range      = 4;
   int count      = 0;
   for( int x = -range; x <= range; ++x )
   {
      for( int y = -range; y <= range; ++y )
      {
         shadow += CalculateShadow( shadowCoords, bias, vec2( x, y ) * texelSize );
         count++;
      }
   }

   return shadow /= count;
}