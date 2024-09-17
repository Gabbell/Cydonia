#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../SHADOW.h"
#include "../TERRAIN.h"
#include "../INSTANCING.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 1 ) uniform INSTANCES { InstancingData instances[MAX_INSTANCES]; };

layout( set = 1, binding = 5 ) uniform sampler2DArray heightMap;

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inTexCoords;

// Interpolators
// =================================================================================================
layout( location = 0 ) out vec3 outUV;
layout( location = 1 ) out float outVSDepth;
layout( location = 2 ) out vec3 outWorldPos;
layout( location = 3 ) out vec3 outNormal;

// =================================================================================================
void main()
{
   const View mainView  = views[0];
   const View lightView = views[1];

   const vec2 size = textureSize( heightMap, 0 ).xy;

   const InstancingData instance = instances[gl_InstanceIndex];

   const vec3 uv = vec3( inTexCoords.xy, instance.index );

   // Normal
   // =============================================================================================
   const vec3 normal = vec3(0.0, 1.0, 0.0); // Basis

   // We use the transpose of the inverse model transformation matrix to
   // have accurate normal directions after non-uniform scaling
   const mat3 normalMat = transpose( inverse( mat3( model ) ) );

   // Displacement
   // =============================================================================================
   const float displacement = GetDisplacement( heightMap, uv );

   // World position to clip position
   // =============================================================================================
   const mat4 instanceModelMat = model * instance.modelMat;

   vec4 pos = vec4( inPosition, 1.0 );
   pos.y += displacement;  // Displace

   const vec4 worldPos = instanceModelMat * pos;
   const vec4 vsPos    = mainView.viewMat * worldPos;

   outUV		= uv;
   outVSDepth	= vsPos.z;
   outWorldPos	= worldPos.xyz;
   outNormal	= normalMat * normal;

   gl_Position = mainView.projMat * vsPos;
}
