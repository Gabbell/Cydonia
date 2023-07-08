#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };
layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

// Inputs & Outputs (Interpolators)
// =================================================================================================
#define DISPLACEMENT_SCALE 10

layout(quads, equal_spacing, cw) in;

layout (location = 0) in vec2 inUV[];
layout (location = 1) in vec3 inNormal[];

// =================================================================================================
void main()
{
	View lightView = views[1];

	// Interpolate UV coordinates
	vec2 uv1 = mix(inUV[0], inUV[1], gl_TessCoord.x);
	vec2 uv2 = mix(inUV[3], inUV[2], gl_TessCoord.x);
	vec2 uv = mix(uv1, uv2, gl_TessCoord.y);

	// Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

	// Displace
	pos.y += texture(heightMap, uv).r * DISPLACEMENT_SCALE;

	// Perspective projection
	vec4 worldPos = model * pos;
	gl_Position = lightView.proj * lightView.view * worldPos;
}