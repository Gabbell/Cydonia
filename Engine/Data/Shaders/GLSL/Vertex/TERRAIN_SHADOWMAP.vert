#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../SHADOW.h"
#include "../TERRAIN.h"
#include "../INSTANCING.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant
{
   mat4 model;
   uint cascade;
};

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 1 ) uniform SHADOWMAPS { ShadowMap shadowMaps[MAX_SHADOW_MAPS]; };
layout( set = 0, binding = 3 ) uniform INSTANCES { InstancingData instances[MAX_INSTANCES]; };

layout( set = 1, binding = 5 ) uniform sampler2DArray heightMap;

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inTexCoords;

// =================================================================================================
void main()
{
   const View lightView = views[1];
   const ShadowMap shadowMapParams = shadowMaps[0];
   const InstancingData instance = instances[gl_InstanceIndex];

   const vec2 size = textureSize( heightMap, 0 ).xy;

   vec3 uv = vec3( inTexCoords.xy, instance.index );

   // Displacement
   // =============================================================================================
   const float displacement = GetDisplacement( heightMap, uv );

   // World position to clip position
   // =============================================================================================
   const mat4 instanceModelMat = model * instance.modelMat;

   vec4 pos = vec4( inPosition, 1.0 );
   pos.y += displacement;  // Displace
   
   const vec4 worldPos = instanceModelMat * pos;

   gl_Position = shadowMapParams.cascades[cascade].worldToLightMatrix * worldPos;
}
