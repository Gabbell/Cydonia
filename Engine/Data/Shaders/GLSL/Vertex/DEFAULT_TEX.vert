#version 450
#extension GL_ARB_separate_shader_objects : enable

// Model transforms
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

// =================================================================================================

// Inputs
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec4 inColor;
layout( location = 2 ) in vec3 inTexCoords;
layout( location = 3 ) in vec3 inNormals;

// Outputs
layout( location = 0 ) out vec4 outColor;
layout( location = 1 ) out vec3 outTexCoord;

// =================================================================================================

void main()
{
   gl_Position = proj * view * model * vec4( inPosition, 1.0 );
   outColor    = inColor;
   outTexCoord = inTexCoords;
}
