#version 450
#extension GL_ARB_separate_shader_objects : enable

// Constant buffer
// =================================================================================================
layout( push_constant ) uniform Epsilon { mat4 model; };

// View and environment (Alpha)
// =================================================================================================
layout( set = 0, binding = 0 ) uniform Alpha
{
   mat4 view;
   mat4 proj;
};

// =================================================================================================

// Inputs
layout( location = 0 ) in vec3 inPosition;
layout( location = 2 ) in vec3 inTexCoord;
layout( location = 3 ) in vec3 inNormal;

// Outputs
layout( location = 0 ) out vec3 outTexCoord;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec3 fragPos;

// =================================================================================================

void main()
{
   fragPos = vec3( model * vec4( inPosition, 1.0 ) );  // World coordinates

   gl_Position = proj * view * vec4( fragPos, 1.0 );

   outTexCoord = inTexCoord;
   outNormal   = vec3( model * vec4( inNormal, 1.0 ) );
}
