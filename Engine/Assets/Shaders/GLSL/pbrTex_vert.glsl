#version 450
#extension GL_ARB_separate_shader_objects : enable

// Constant buffer
// =================================================================================================
layout( push_constant ) uniform Epsilon { mat4 model; };

// View and environment
// =================================================================================================
layout( set = 0, binding = 0 ) uniform Alpha
{
   mat4 view;
   mat4 proj;
};

// =================================================================================================

layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

// Inputs
layout( location = 0 ) in vec3 inPosition;
layout( location = 2 ) in vec3 inTexCoord;
layout( location = 3 ) in vec3 inNormal;

// Outputs
layout( location = 0 ) out vec3 outTexCoord;
layout( location = 1 ) out vec3 outNormal;
layout( location = 2 ) out vec3 fragPos;

const float heightModulator = 1.0;

// =================================================================================================

void main()
{
   fragPos     = vec3( model * vec4( inPosition, 1.0 ) );  // World coordinates
   vec3 normal = normalize( mat3( model ) * inNormal );

   // Pulling UP
   float heightValue = texture( heightMap, inTexCoord.xy ).r;
   fragPos += heightModulator * normal * heightValue;

   gl_Position = proj * view * vec4( fragPos, 1.0 );

   outTexCoord = inTexCoord;
   outNormal   = normal;
}
