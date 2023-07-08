#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"

layout( set = 1, binding = 5 ) uniform sampler2D disp;

// Constant buffer
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 inTexCoords;
layout( location = 3 ) in vec4 inColor;  // Not used

// Interpolators
// =================================================================================================
layout( location = 0 ) out vec2 outUV;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec3 outWorldPos;
layout( location = 3 ) out vec4 outShadowCoord;

// =================================================================================================
// Bump mapping based on heightmap
vec3 getNormals( vec2 uv )
{
   const vec2 texelSize = 1.0 / textureSize( disp, 0 );
   const ivec3 off      = ivec3( -1, 0, 1 );
   const vec3 size      = vec3( 2.0 * texelSize.x, 2.0 * texelSize.y, 0.0 );

   // Uses 4 samples to create one normal, effectively "blurring" the normals
   // I don't think these are scaled properly, noon looks weird
   float hL = textureOffset( disp, uv, off.xy ).r;
   float hR = textureOffset( disp, uv, off.zy ).r;
   float hU = textureOffset( disp, uv, off.yx ).r;
   float hD = textureOffset( disp, uv, off.yz ).r;

   vec3 va = vec3( size.x, hR - hL, size.z );
   vec3 vb = vec3( size.z, hU - hD, -size.y );

   /*
   // 3 samples variant
   float hO = texture(disp, uv).r;
   float hR = textureOffset( disp, uv, off.zy ).r;
   float hU = textureOffset( disp, uv, off.yx ).r;

   vec3 va = (vec3(size.x, hR - hO, size.y));
   vec3 vb = (vec3(size.y, hU - hO, -size.x));
   */

   return normalize( cross( va, vb ) );
}

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

   outUV     = inTexCoords.xy;
   outNormal = getNormals( inTexCoords.xy );

   // Y only displacement for now
   float displacement      = clamp( texture( disp, inTexCoords.xy ).r, 0.0, 1.0 );
   float displacementScale = 10.0f;

   outWorldPos = mat3( model ) * inPosition;           // World coordinates
   outWorldPos.y += displacement * displacementScale;  // Apply displacement

   outShadowCoord = biasMat * lightView.proj * lightView.view * vec4( outWorldPos, 1.0 );
   gl_Position    = mainView.proj * mainView.view * vec4( outWorldPos, 1.0 );
}
