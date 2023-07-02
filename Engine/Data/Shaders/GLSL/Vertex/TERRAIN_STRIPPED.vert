#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( set = 1, binding = 5 ) uniform sampler2D disp;

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
}
view;

layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 inTexCoords;
layout( location = 3 ) in vec4 inColor;

void main()
{
   // Y only displacement for now
   float displacement      = clamp( texture( disp, inTexCoords.xy ).r, 0.0, 1.0 );
   float displacementScale = 10.0f;

   vec3 worldPos = mat3( model ) * inPosition;      // World coordinates
   worldPos.y += displacement * displacementScale;  // Apply displacement

   gl_Position = view.proj * view.view * vec4( worldPos, 1.0 );
}