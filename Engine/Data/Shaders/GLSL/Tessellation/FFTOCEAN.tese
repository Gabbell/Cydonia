#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../NOISE.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

// Inputs & Outputs (Interpolators)
// =================================================================================================
layout( quads, equal_spacing, cw ) in;

layout( location = 0 ) in vec2 inUV[];

layout( location = 0 ) out vec2 outUV;
layout( location = 1 ) out vec3 outWorldPos;
layout( location = 2 ) out vec4 outShadowCoord;

// Transforms NDC space [-1, 1] in xy to UV space [0, 1]
// clang-format off
const mat4 biasMat = mat4( 0.5, 0.0, 0.0, 0.0,
                           0.0, 0.5, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           0.5, 0.5, 0.0, 1.0 );
// clang-format on

// =================================================================================================
void main()
{
   const View mainView  = views[0];
   const View lightView = views[1];

   // Interpolate UV coordinates
   vec2 uv1 = mix( inUV[0], inUV[1], gl_TessCoord.x );
   vec2 uv2 = mix( inUV[3], inUV[2], gl_TessCoord.x );
   outUV    = mix( uv1, uv2, gl_TessCoord.y );

   vec3 displacement = texture( heightMap, outUV.xy ).xyz;

   // Interpolate positions
   vec4 pos1 = mix( gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x );
   vec4 pos2 = mix( gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x );
   vec4 pos  = mix( pos1, pos2, gl_TessCoord.y );

   // Displace
   pos.xyz += displacement;

   // Perspective projection
   vec4 worldPos = model * pos;
   gl_Position   = mainView.proj * mainView.view * worldPos;

   outWorldPos    = worldPos.xyz;
   outShadowCoord = biasMat * lightView.proj * lightView.view * worldPos;
}