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

// Bump mapping based on heightmap
vec3 getNormals( vec2 uv )
{
   const vec2 texelSize = 1.0 / textureSize( heightMap, 0 );
   const ivec3 off      = ivec3( -1, 0, 1 );
   const vec3 size      = vec3( 2.0 * texelSize.x, 2.0 * texelSize.y, 0.0 );

   // Uses 4 samples to create one normal, effectively "blurring" the normals
   // I don't think these are scaled properly, noon looks weird
   float hL = textureOffset( heightMap, uv, off.xy ).r;
   float hR = textureOffset( heightMap, uv, off.zy ).r;
   float hU = textureOffset( heightMap, uv, off.yx ).r;
   float hD = textureOffset( heightMap, uv, off.yz ).r;

   vec3 va = vec3( size.x, hR - hL, size.z );
   vec3 vb = vec3( size.z, hU - hD, -size.y );

   /*
   // 3 samples variant
   float hO = texture(heightMap, uv).r;
   float hR = textureOffset( heightMap, uv, off.zy ).r;
   float hU = textureOffset( heightMap, uv, off.yx ).r;

   vec3 va = (vec3(size.x, hR - hO, size.y));
   vec3 vb = (vec3(size.y, hU - hO, -size.x));
   */

   return normalize( cross( va, vb ) );
}

// =================================================================================================
void main()
{
	View mainView = views[0];
	View lightView = views[1];

	// Interpolate UV coordinates
	vec2 uv1 = mix(inUV[0], inUV[1], gl_TessCoord.x);
	vec2 uv2 = mix(inUV[3], inUV[2], gl_TessCoord.x);
	outUV = mix(uv1, uv2, gl_TessCoord.y);

	vec3 n1 = mix(inNormal[0], inNormal[1], gl_TessCoord.x);
	vec3 n2 = mix(inNormal[3], inNormal[2], gl_TessCoord.x);
	outNormal = getNormals(outUV);

	// Interpolate positions
	vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
	vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
	vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

	// Displace
	pos.y += texture(heightMap, outUV).r * DISPLACEMENT_SCALE;

	// Perspective projection
	vec4 worldPos = model * pos;
	gl_Position = mainView.proj * mainView.view * worldPos;

	outWorldPos = worldPos.xyz;
	outShadowCoord = biasMat * lightView.proj * lightView.view * worldPos;
}