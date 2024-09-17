#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../INSTANCING.h"

layout( push_constant ) uniform PushModel { layout( offset = 0 ) mat4 modelMat; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 1 ) uniform INSTANCES { InstancingData instances[MAX_INSTANCES]; };

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
layout( location = 2 ) out vec3 outWorldPos;

// =================================================================================================
void main()
{
   const View mainView = views[0];

   const mat4x4 instanceModelMat = modelMat * mat4( instances[gl_InstanceIndex].modelMat );
   vec3 worldPos = vec3( instanceModelMat * vec4( inPosition, 1.0 ) );  // World coordinates

   outTexCoord = inTexCoord.xy;
   outNormal   = normalize( vec3( instanceModelMat * vec4( inNormal, 0.0 ) ) );
   outWorldPos = worldPos;

   gl_Position = mainView.projMat * mainView.viewMat * vec4( worldPos, 1.0 );
}
