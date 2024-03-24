#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant
{
   mat4 model;
};

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 1 ) uniform DEBUG { vec4 color; };

layout( location = 0 ) in vec3 inPosition;

layout( location = 0 ) out vec4 outColor;

void main()
{
   const View mainView = views[0];

   outColor = color;

   gl_Position = mainView.projMat * mainView.viewMat * model * vec4( inPosition, 1.0 );
}