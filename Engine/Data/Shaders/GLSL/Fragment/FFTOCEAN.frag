#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "LIGHTING.h"
#include "../VIEW.h"
#include "../NOISE.h"

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };
layout( set = 1, binding = 0 ) uniform Lights { Light lights[MAX_LIGHTS]; };
layout( set = 1, binding = 1 ) uniform sampler2DShadow shadowMap;
layout( set = 1, binding = 2 ) uniform sampler2D normalMap;
layout( set = 1, binding = 5 ) uniform sampler2D displacementMap;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inWorldPos;
layout( location = 2 ) in vec4 inShadowCoord;

layout( location = 0 ) out vec4 outColor;

float diffuse( vec3 n, vec3 l, float p ) { return pow( dot( n, l ) * 0.4 + 0.6, p ); }

float specular( vec3 n, vec3 l, vec3 e, float s )
{
   return pow( max( dot( reflect( e, n ), l ), 0.0 ), s );
}

const vec3 SEA_BASE        = vec3( 0.0, 0.09, 0.18 );
const vec3 SEA_WATER_COLOR = vec3( 0.8, 0.9, 0.6 ) * 0.6;

// =================================================================================================
void main()
{
   const View mainView  = views[0];
   const Light curLight = lights[0];

   // Vectors
   // ============================================================================================
   const vec4 D = texture( displacementMap, inUV );
   const vec3 L = normalize( -curLight.direction.xyz );
   const vec3 N = normalize( vec3( 0.0, 1.0, 0.0 ) - vec3( D.x, 0.0, D.z ) );
   const vec3 V = normalize( inWorldPos - mainView.pos.xyz );

   // Factors
   // ============================================================================================
   const float fakeFresnel = pow( clamp( 1.0 - dot( N, -V ), 0.0, 1.0 ), 1.0 );
   const float shadow      = ShadowPCF( shadowMap, inShadowCoord );

   // Foam
   // ============================================================================================
   // The Jacobian determinant is in the alpha of the displacement map
   const float jacobianBias = 0.8;
   const float foamFactor   = clamp( jacobianBias - D.a, 0.0, 1.0 );

   // Color
   // ============================================================================================
   const vec3 radiance = curLight.color.rgb;

   // Diffuse
   const vec3 reflected = vec3( 0.53, 0.81, 0.92 ) * 0.9;
   const vec3 refracted = SEA_BASE + diffuse( N, L, 80.0 ) * SEA_WATER_COLOR * 0.12;

   vec3 color = mix( refracted, reflected, fakeFresnel );

   // SSS
   color += max( dot( L, V ), 0.0 ) * SEA_WATER_COLOR * max( inWorldPos.y - 0.6, 0.0 ) * 0.33;

   // Foam
   color += vec3( 1.0, 1.0, 1.0 ) * foamFactor;

   // Specular
   color += vec3( specular( N, L, V, 60.0 ) ) * shadow;

   outColor = vec4( color, 1.0 );
}
