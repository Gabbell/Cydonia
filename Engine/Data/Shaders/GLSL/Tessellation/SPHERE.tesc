#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../TESSELLATION.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };
layout( set = 0, binding = 2 ) uniform TesselationParams { TessellationParams params; };

// Inputs & Outputs (Interpolators)
// =================================================================================================
#define VERTICES_PER_PATCH 3

layout( vertices = VERTICES_PER_PATCH ) out;

// Functions
// =================================================================================================

// Calculate the tessellation factor based on distance to sphere's radius
float getTessFactor()
{
   const View mainView = views[0];

   const vec4 center  = model * vec4( 0.0, 0.0, 0.0, 1.0 );
   const float radius = model[0][0];  // This assumes equal scaling in all axes

   vec4 v0 = mainView.view * center;

   vec4 clip0 = mainView.proj * ( v0 - vec4( radius, vec3( 0.0 ) ) );
   vec4 clip1 = mainView.proj * ( v0 + vec4( radius, vec3( 0.0 ) ) );

   clip0 /= clip0.w;
   clip1 /= clip1.w;
   clip0.xy *= params.viewportDims;
   clip1.xy *= params.viewportDims;

   return clamp(
       distance( clip0, clip1 ) / params.tessellatedEdgeSize * params.tessellationFactor,
       1.0,
       128.0 );
}

// Checks the current's patch visibility against the frustum using a sphere check
// Sphere radius is given by the patch size
bool frustumCheck()
{
   // Fixed radius (increase if patch size is increased)
   const float radius = 0.0f;
   vec4 pos           = model * normalize( gl_in[gl_InvocationID].gl_Position );

   // Check sphere against frustum planes
   for( int i = 0; i < 6; i++ )
   {
      if( dot( pos, params.frustumPlanes[i] ) + radius < 0.0 )
      {
         return false;
      }
   }
   return true;
}

// Main
// =================================================================================================
void main()
{
   if( gl_InvocationID == 0 )
   {
      if( false )
      {
         gl_TessLevelInner[0] = 0.0;
         gl_TessLevelInner[1] = 0.0;
         gl_TessLevelOuter[0] = 0.0;
         gl_TessLevelOuter[1] = 0.0;
         gl_TessLevelOuter[2] = 0.0;
      }
      else
      {
         if( params.tessellationFactor > 0.0 )
         {
            float tessLevel = getTessFactor();

            gl_TessLevelOuter[0] = tessLevel;
            gl_TessLevelOuter[1] = tessLevel;
            gl_TessLevelOuter[2] = tessLevel;
            gl_TessLevelInner[0] = tessLevel;
            gl_TessLevelInner[1] = tessLevel;
         }
         else
         {
            // Tessellation factor can be set to zero by example
            // to demonstrate a simple passthrough
            gl_TessLevelInner[0] = 1.0;
            gl_TessLevelInner[1] = 1.0;
            gl_TessLevelOuter[0] = 1.0;
            gl_TessLevelOuter[1] = 1.0;
            gl_TessLevelOuter[2] = 1.0;
         }
      }
   }

   gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}