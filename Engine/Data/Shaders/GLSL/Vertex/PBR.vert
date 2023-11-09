#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"

layout( push_constant ) uniform PushModel { layout( offset = 0 ) mat4 modelMat; };

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inTexCoord;
layout( location = 2 ) in vec3 inNormal;
layout( location = 3 ) in vec3 inTangent;

// Interpolators
// =================================================================================================
layout( location = 0 ) out vec2 outUV;
layout( location = 1 ) out vec3 outWorldPos;
layout( location = 2 ) out vec3 outShadowCoord;
layout( location = 3 ) out mat3 outTBN;

// =================================================================================================
// Transforms NDC space [-1, 1] in xy to UV space [0, 1]
// clang-format off
const mat4 biasMat = mat4( 0.5, 0.0, 0.0, 0.0,
                           0.0, 0.5, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           0.5, 0.5, 0.0, 1.0 );
// clang-format on

void main()
{
   const View mainView  = views[0];
   const View lightView = views[1];

   const vec4 worldPos = modelMat * vec4( inPosition, 1.0 );  // World coordinates

   // We use the transpose of the inverse model transformation matrix to
   // have accurate normal directions after non-uniform scaling
   const mat3 normalMat = transpose( inverse( mat3( modelMat ) ) );

   const vec3 N = normalize( normalMat * inNormal );
   vec3 T       = normalize( normalMat * inTangent );
   T            = normalize( T - dot( T, N ) * N );  // Reorthogonalize
   const vec3 B = cross( N, T );

   vec4 shadowCoord = biasMat * lightView.proj * lightView.view * worldPos;

   outUV          = vec2( inTexCoord.x, 1.0 - inTexCoord.y );
   outWorldPos    = worldPos.xyz;
   outShadowCoord = shadowCoord.xyz / shadowCoord.w;
   outTBN         = mat3( T, B, N );

   gl_Position = mainView.proj * mainView.view * worldPos;
}
