#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "TESSELLATION.h"

// Constant Buffers & Uniforms
// =================================================================================================
layout( push_constant ) uniform PushConstant { mat4 model; };

layout( set = 0, binding = 0 ) uniform VIEWS { View views[MAX_VIEWS]; };
layout( set = 0, binding = 2 ) uniform TESSELATION { TessellationParams params; };
layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

// Inputs & Outputs (Interpolators)
// =================================================================================================
#define VERTICES_PER_PATCH 4

layout( vertices = VERTICES_PER_PATCH ) out;

layout( location = 0 ) in vec2 inUV[];

layout( location = 0 ) out vec2 outUV[VERTICES_PER_PATCH];

// Functions
// =================================================================================================

// Calculate the tessellation factor based on screen space
// dimensions of the edge
float screenSpaceTessFactor( vec4 p0, vec4 p1 )
{
   const View mainView = views[0];

   // Calculate edge mid point
   vec4 midPoint = 0.5 * ( p0 + p1 );

   // Sphere radius as distance between the control points
   float radius = distance( p0, p1 ) / 2.0;

   // View space
   vec4 v0 = mainView.view * model * midPoint;

   // Project into clip space
   vec4 clip0 = ( mainView.proj * ( v0 - vec4( radius, vec3( 0.0 ) ) ) );
   vec4 clip1 = ( mainView.proj * ( v0 + vec4( radius, vec3( 0.0 ) ) ) );

   // Get normalized device coordinates
   clip0 /= clip0.w;
   clip1 /= clip1.w;

   // Convert to viewport coordinates
   clip0.xy *= params.viewportDims;
   clip1.xy *= params.viewportDims;

   // Return the tessellation factor based on the screen size
   // given by the distance of the two edge control points in screen space
   // and a reference (min.) tessellation size for the edge set by the application
   return clamp(
       distance( clip0, clip1 ) / params.tessellatedEdgeSize * params.tessellationFactor,
       1.0,
       64.0 );
}

// Checks the current's patch visibility against the frustum using a sphere check
// Sphere radius is given by the patch size
bool frustumCheck()
{
   // Fixed radius (increase if patch size is increased)
   const float radius = 8.0f;
   vec4 pos           = gl_in[gl_InvocationID].gl_Position;
   pos.xyz += texture( heightMap, inUV[0] ).xyz;
   pos = model * pos;
   
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
/* ASCII Representation of Inner (I) and Outer (O) tesselation levels

            O-3
(0, 1)________________ (1, 1)
     |	_________   |
     |  |         |  |
     |  |   I-0   |  |
 O-0 |  |I-1   I-1|  | O-2
     |  |   I-0   |  |
     |  |_________|  |
     |_______________|
(0, 0)                 (1, 0)
            O-1
*/

void main()
{
   if( gl_InvocationID == 0 )
   {
      // Culling is wrong at steep angles
      if( false /*!frustumCheck()*/ )
      {
         gl_TessLevelInner[0] = 0.0;
         gl_TessLevelInner[1] = 0.0;
         gl_TessLevelOuter[0] = 0.0;
         gl_TessLevelOuter[1] = 0.0;
         gl_TessLevelOuter[2] = 0.0;
         gl_TessLevelOuter[3] = 0.0;
      }
      else
      {
         if( params.tessellationFactor > 0.0 )
         {
            gl_TessLevelOuter[0] =
                screenSpaceTessFactor( gl_in[3].gl_Position, gl_in[0].gl_Position );
            gl_TessLevelOuter[1] =
                screenSpaceTessFactor( gl_in[0].gl_Position, gl_in[1].gl_Position );
            gl_TessLevelOuter[2] =
                screenSpaceTessFactor( gl_in[1].gl_Position, gl_in[2].gl_Position );
            gl_TessLevelOuter[3] =
                screenSpaceTessFactor( gl_in[2].gl_Position, gl_in[3].gl_Position );
            gl_TessLevelInner[0] = mix( gl_TessLevelOuter[0], gl_TessLevelOuter[3], 0.5 );
            gl_TessLevelInner[1] = mix( gl_TessLevelOuter[2], gl_TessLevelOuter[1], 0.5 );
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
            gl_TessLevelOuter[3] = 1.0;
         }
      }
   }

   gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
   outUV[gl_InvocationID]              = inUV[gl_InvocationID];
}