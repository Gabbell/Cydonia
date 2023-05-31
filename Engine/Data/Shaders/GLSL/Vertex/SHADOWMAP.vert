#version 450
#extension GL_ARB_separate_shader_objects : enable

// Constant buffer
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

// View and environment (Alpha)
// =================================================================================================
layout( set = 0, binding = 0 ) uniform EnvironmentView
{
   vec4 pos;
   mat4 view;
   mat4 proj;
};

layout( set = 0, binding = 1 ) uniform Lights
{
   mat4 viewMat;
   vec4 position;
   vec4 color;
   bool enabled;
}
lights;

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec4 inColor;
layout( location = 2 ) in vec3 inTexCoords;
layout( location = 3 ) in vec3 inNormals;

layout( location = 0 ) out vec4 outColor;

void main()
{
   gl_Position = proj * view * model * vec4( inPosition, 1.0 );
   outColor    = inColor;
}