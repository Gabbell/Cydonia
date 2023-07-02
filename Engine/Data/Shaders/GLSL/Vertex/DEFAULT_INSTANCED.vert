#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "INSTANCING.h"

layout( push_constant ) uniform PushModel { layout( offset = 0 ) mat4 modelMat; };

layout( set = 0, binding = 0 ) uniform EnvironmentView
{
   vec4 pos;
   mat4 viewMat;
   mat4 projMat;
};

// Vertex Inputs
// =================================================================================================
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 inTexCoord;
layout( location = 3 ) in vec4 inColor;  // Not used

// Interpolators
// =================================================================================================
layout( location = 0 ) out vec2 outTexCoord;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec3 worldPos;
layout( location = 3 ) out vec3 camPos;

// =================================================================================================
void main()
{
   mat4x4 finalModelMat = modelMat * mat4( instances[gl_InstanceIndex].modelMat );

   worldPos = vec3( finalModelMat * vec4( inPosition, 1.0 ) );  // World coordinates
   camPos   = vec3( pos );

   outTexCoord = inTexCoord.xy;
   outNormal   = normalize( vec3( finalModelMat * vec4( inNormal, 0.0 ) ) );

   gl_Position = projMat * viewMat * vec4( worldPos, 1.0 );
}
