#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../SHADOW.h"
#include "../INSTANCING.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 2 ) uniform INSTANCES { InstancingData instances[MAX_INSTANCES]; };

layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

// Inputs & Outputs (Interpolators)
// =================================================================================================
layout( quads, equal_spacing, cw ) in;

layout( location = 0 ) in vec3 inUV[];

layout( location = 0 ) out vec3 outUV;
layout( location = 1 ) out vec3 outVSPos;
layout( location = 2 ) out vec3 outWorldPos;
layout( location = 3 ) out vec4 outShadowCoords;

// =================================================================================================
void main()
{
   // Interpolate UV coordinates
   vec3 uv1 = mix( inUV[0], inUV[1], gl_TessCoord.x );
   vec3 uv2 = mix( inUV[3], inUV[2], gl_TessCoord.x );
   outUV    = mix( uv1, uv2, gl_TessCoord.y );

   vec3 displacement = texture( heightMap, outUV.xy ).xyz;

   // Interpolate positions
   vec4 pos1 = mix( gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x );
   vec4 pos2 = mix( gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x );
   vec4 pos  = mix( pos1, pos2, gl_TessCoord.y );

   // Displace
   pos.xyz += displacement;

   // World position to clip position
   // =============================================================================================
   const View mainView = views[0];

   const uint instanceIndex    = uint( inUV[0].z );
   const mat4 instanceModelMat = model * instances[instanceIndex].modelMat;
   const vec4 worldPos         = instanceModelMat * pos;
   const vec4 vsPos			   = mainView.viewMat * worldPos;

   // Outputs
   // =============================================================================================
   outVSPos	       = vsPos.xyz;
   outWorldPos     = worldPos.xyz;

   gl_Position = mainView.projMat * vsPos;
}