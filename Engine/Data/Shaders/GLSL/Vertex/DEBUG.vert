#version 450
#extension GL_ARB_separate_shader_objects : enable

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

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inTexCoords;  // Unused

layout( location = 0 ) out vec4 outColor;

void main()
{
   gl_Position = proj * view * model * vec4( inPosition, 1.0 );
   outColor    = vec4(1.0, 0.0, 0.0, 1.0);
}