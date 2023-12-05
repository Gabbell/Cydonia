#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../INSTANCING.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 2 ) uniform INSTANCES { InstancingData instances[MAX_INSTANCES]; };

layout( set = 1, binding = 5 ) uniform sampler2DArray heightMap;

// Inputs & Outputs (Interpolators)
// =================================================================================================
layout( quads, equal_spacing, cw ) in;

layout( location = 0 ) in vec3 inUV[];
layout( location = 1 ) flat in uint inInstanceIndex[];

// =================================================================================================
void main()
{
   View lightView = views[1];

   // Interpolate UV coordinates
   const vec3 uv1 = mix( inUV[0], inUV[1], gl_TessCoord.x );
   const vec3 uv2 = mix( inUV[3], inUV[2], gl_TessCoord.x );
   vec3 uv  = mix( uv1, uv2, gl_TessCoord.y );

      const vec2 size = textureSize(heightMap, 0).xy;
   uv.xy = uv.xy * ((size - 1.0) / size) + (0.5 / size); // Snapped UV

   const float displacement = texture( heightMap, uv ).r;

   // Interpolate positions
   const vec4 pos1 = mix( gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x );
   const vec4 pos2 = mix( gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x );
   vec4 pos        = mix( pos1, pos2, gl_TessCoord.y );

   // World position and perspective projection
   const uint instanceIndex    = inInstanceIndex[0];
   const mat4 instanceModelMat = model * instances[instanceIndex].modelMat;
   pos.y += displacement;  // Displace
   vec4 worldPos = instanceModelMat * pos;
   gl_Position   = lightView.proj * lightView.view * worldPos;
}