#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../SHADOW.h"

layout( push_constant ) uniform PushConstant { layout( offset = 0 ) mat4 model; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inTexCoord;
layout( location = 2 ) in vec3 inNormal;
layout( location = 3 ) in vec3 inTangent;

// Interpolators
// =================================================================================================
layout( location = 0 ) out vec2 outUV;
layout( location = 1 ) out vec3 outVSPos;
layout( location = 2 ) out vec3 outWorldPos;
layout( location = 3 ) out vec3 outTSViewDir;
layout( location = 4 ) out mat3 outTBN;

// =================================================================================================
void main()
{
   const View mainView  = views[0];
   const View lightView = views[1];

   const vec4 worldPos	= model * vec4( inPosition, 1.0 );  // World coordinates
   const vec4 vsPos		= mainView.viewMat * worldPos;

   // We use the transpose of the inverse model transformation matrix to
   // have accurate normal directions after non-uniform scaling
   const mat3 normalMat = transpose( inverse( mat3( model ) ) );

   const vec3 N = normalize( normalMat * inNormal );
   vec3 T       = normalize( normalMat * inTangent );
   T            = normalize( T - dot( T, N ) * N );  // Reorthogonalize
   const vec3 B = cross( N, T );

   const mat3 TBN                 = mat3( T, B, N );
   const mat3 worldToTangentSpace = transpose( TBN );

   outUV		= vec2( inTexCoord.x, 1.0 - inTexCoord.y );
   outVSPos		= vsPos.xyz;
   outWorldPos	= worldPos.xyz;
   outTSViewDir	= worldToTangentSpace * ( worldPos.xyz - mainView.pos.xyz );
   outTBN		= TBN;

   gl_Position = mainView.projMat * vsPos;
}
