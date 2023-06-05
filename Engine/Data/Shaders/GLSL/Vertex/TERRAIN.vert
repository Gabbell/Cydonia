#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../Fragment/MATERIAL.h"

layout( set = MATERIAL_SET, binding = DISPLACEMENT_SLOT ) uniform sampler2D disp;

// Constant buffer
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

// View and environment
// =================================================================================================
layout( set = 0, binding = 0 ) uniform EnvironmentView
{
   vec4 pos;
   mat4 view;
   mat4 proj;
};

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 inTexCoord;
layout( location = 3 ) in vec4 inColor;  // Not used

// Interpolators
// =================================================================================================
layout( location = 0 ) out vec2 outUV;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec3 worldPos;
layout( location = 3 ) out vec3 camPos;

// =================================================================================================
void main()
{
   camPos = vec3( pos );

   outUV     = inTexCoord.xy;
   outNormal = normalize( vec3( model * vec4( inNormal, 0.0 ) ) );

   // Y only displacement for now
   float displacement      = clamp( texture( disp, inTexCoord.xy ).r, 0.0, 1.0 );
   float displacementScale = 10.0f;

   worldPos = vec3( model * vec4( inPosition, 1.0 ) );  // World coordinates
   worldPos.y += displacement * displacementScale; // Apply displacement

   gl_Position = proj * view * vec4( worldPos, 1.0 );
}
