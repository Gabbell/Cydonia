#version 450
#extension GL_ARB_separate_shader_objects : enable

// View and environment
// =================================================================================================
layout( set = 0, binding = 0 ) uniform Alpha
{
    mat4 view;
    mat4 proj;
};

// Shader control values
// =================================================================================================
layout( set = 1, binding = 0 ) uniform Beta
{
    vec4 control;
};

// Model transforms
// =================================================================================================
layout( push_constant ) uniform Epsilon
{
    mat4 model;
};

// =================================================================================================

// Inputs
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec4 inColor;
layout( location = 2 ) in vec3 inTexCoord;
layout( location = 3 ) in vec3 inNormal;

// Outputs
layout( location = 0 ) out vec4 outColor;
layout( location = 1 ) out vec3 outTexCoord;
layout( location = 2 ) out vec3 outNormal;
layout( location = 3 ) out vec3 viewDir;
layout( location = 4 ) out vec3 fragPos;

// =================================================================================================

void main()
{
	 fragPos = vec3( model * vec4( inPosition, 1.0 ) );
	 viewDir = vec3( view * vec4( fragPos, 1.0 ) );

    gl_Position = proj * vec4( viewDir, 1.0 );

    outColor = inColor;
    outTexCoord = inTexCoord;
    outNormal = inNormal;
}

