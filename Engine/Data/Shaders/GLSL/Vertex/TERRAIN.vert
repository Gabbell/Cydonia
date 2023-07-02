#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( set = 1, binding = 5 ) uniform sampler2D disp;

// Constant buffer
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

// View and environment
// =================================================================================================
struct View
{
   vec4 pos;
   mat4 view;
   mat4 proj;
};

#define MAX_VIEWS 8
layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };

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
layout( location = 3 ) out vec3 outCamPos;
layout( location = 4 ) out vec4 outShadowCoord;

// =================================================================================================
// Bump mapping based on heightmap
vec3 getNormals( vec2 uv )
{
   const vec2 texelSize = 1.0 / textureSize( disp, 0 );
   const vec2 size      = vec2( 2.0 * 2.0 * texelSize.x, 0.0 );
   const ivec3 off      = ivec3( -1, 0, 1 );

   float hL = textureOffset( disp, uv, off.xy ).r;
   float hR = textureOffset( disp, uv, off.zy ).r;
   float hU = textureOffset( disp, uv, off.yx ).r;
   float hD = textureOffset( disp, uv, off.yz ).r;

   vec3 va = vec3( size.x, hR - hL, size.y );
   vec3 vb = vec3( size.y, hU - hD, -size.x );

/*
   // 3 samples variant
   float hO = texture(disp, uv).r;
   float hR = textureOffset( disp, uv, off.zy ).r;
   float hU = textureOffset( disp, uv, off.yx ).r;

   vec3 va = (vec3(size.x, hR - hO, size.y));
   vec3 vb = (vec3(size.y, hU - hO, -size.x));
*/

   // In the case of a rectangular texture, the Y component might not be texelSize.x but
   // something adjusted by the aspect ratio?
   return normalize( cross( va, vb ) );
}

// Transforms NDC space [-1, 1] in xy to UV space [0, 1]
const float depthBias = -0.005;

// clang-format off
const mat4 biasMat = mat4( 0.5, 0.0, 0.0, 0.0,
                           0.0, 0.5, 0.0, 0.0,
                           0.0, 0.0, 1.0, 0.0,
                           0.5, 0.5, depthBias, 1.0 );
// clang-format on

// =================================================================================================
void main()
{
   const View mainView  = views[0];
   const View lightView = views[1];

   outCamPos = vec3( mainView.pos );

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
