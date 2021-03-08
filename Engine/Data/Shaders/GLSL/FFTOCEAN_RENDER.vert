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

layout( set = 0, binding = 1 ) uniform sampler2D disp;

// =================================================================================================

// Inputs
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec4 inColor;
layout( location = 2 ) in vec3 inUV;
layout( location = 3 ) in vec3 inNormal;

// Outputs
layout( location = 0 ) out vec4 outColor;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec3 outUV;
layout( location = 3 ) out vec3 fragPos;
layout( location = 4 ) out vec3 viewPos;

// =================================================================================================

void main()
{
   // Calculating vertex displacement by sampling from the map
   const vec3 displacement = texture( disp, inUV.xy ).rgb;

   // Passing normal and UV directly to fragment
   outNormal = normalize( inNormal );
   outUV     = inUV;

   // Calculating the fragment position in world coordinates and setting the camera position for
   // calculations in the fragment shader
   fragPos = vec3( model * vec4( displacement + inPosition, 1.0 ) );
   viewPos = vec3( pos );

   gl_Position = proj * view * vec4( fragPos, 1.0 );
   outColor    = inColor;
}
