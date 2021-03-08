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

layout( set = 1, binding = 5 ) uniform sampler2D displacement;

// =================================================================================================

// Inputs
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec4 inColor;  // Not used
layout( location = 2 ) in vec3 inTexCoord;
layout( location = 3 ) in vec3 inNormal;

// Outputs
layout( location = 0 ) out vec3 outTexCoord;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec3 fragPos;
layout( location = 3 ) out vec3 viewPos;

// =================================================================================================

void main()
{
   const vec3 displacement = texture( displacement, inTexCoord.xy ).rgb;

   fragPos = vec3( model * vec4( displacement + inPosition, 1.0 ) );  // World coordinates
   viewPos = vec3( pos );

   outTexCoord = inTexCoord;
   outNormal   = normalize( vec3( model * vec4( inNormal, 1.0 ) ) );

   gl_Position = proj * view * vec4( fragPos, 1.0 );
}
