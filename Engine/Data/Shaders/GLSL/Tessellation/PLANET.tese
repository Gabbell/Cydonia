#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
//layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

// Inputs & Outputs (Interpolators)
// =================================================================================================
layout( triangles, equal_spacing, ccw ) in;

layout( location = 0 ) out vec2 outUV;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec3 outWorldPos;
layout( location = 3 ) out vec4 outShadowCoord;

// Transforms NDC space [-1, 1] in xy to UV space [0, 1]
// clang-format off
const mat4 biasMat = mat4( 0.5, 0.0, 0.0, 0.0,
                           0.0, 0.5, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           0.5, 0.5, 0.0, 1.0 );
// clang-format on

const float PI                 = 3.141592653589793;
const float DISPLACEMENT_SCALE = 10.0;

// =================================================================================================
void main()
{
   const View mainView  = views[0];
   const View lightView = views[1];

   // Barycentric interpolation
   vec4 pos = gl_TessCoord.x * gl_in[0].gl_Position + gl_TessCoord.y * gl_in[1].gl_Position +
              gl_TessCoord.z * gl_in[2].gl_Position;

   outUV = vec2( 0.5 + atan( pos.z, pos.x ) / ( 2.0 * PI ), 0.5 + asin( pos.y ) / PI );

   outNormal = normalize( model * pos ).xyz;

   // Displace
   const vec3 normal = normalize( pos.xyz );
   pos.xyz           = normal;

   // Perspective projection
   vec4 worldPos = model * pos;
   //worldPos.xyz += normal * texture( heightMap, outUV ).r;
   gl_Position   = mainView.proj * mainView.view * worldPos;

   outWorldPos    = worldPos.xyz;
   outShadowCoord = biasMat * lightView.proj * lightView.view * worldPos;
}