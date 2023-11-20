#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../LIGHTING.h"

layout( set = 0, binding = 1 ) uniform sampler2DShadow shadowMap;

layout( set = 1, binding = 0 ) uniform sampler2D albedo;
layout( set = 1, binding = 1 ) uniform sampler2D normals;
layout( set = 1, binding = 2 ) uniform sampler2D metalness;
layout( set = 1, binding = 3 ) uniform sampler2D roughness;
layout( set = 1, binding = 4 ) uniform sampler2D ambientOcclusion;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inWorldPos;
layout( location = 2 ) in vec3 inShadowCoord;
layout( location = 3 ) in mat3 inTBN;

layout( location = 0 ) out vec4 outAlbedo;
layout( location = 1 ) out vec4 outNormal;
layout( location = 2 ) out vec4 outPBR;
layout( location = 3 ) out vec4 outShadow;
layout( location = 4 ) out float outDepth;

// =================================================================================================
void main()
{
   const vec3 albedo     = texture( albedo, inUV ).rgb;
   const vec3 TSNormal   = texture( normals, inUV ).xyz * 2.0 - 1.0;  // Tangent space
   const float metalness = texture( metalness, inUV ).r;
   const float roughness = texture( roughness, inUV ).r;
   const float ao        = texture( ambientOcclusion, inUV ).r;

   const vec3 normal = normalize( inTBN * normalize( TSNormal ) );

   outAlbedo = vec4( albedo, 1.0 );
   outNormal = vec4( normal, 1.0 );
   outPBR    = vec4( roughness, metalness, ao, 0.0 );
   outShadow = vec4( ShadowPCF( shadowMap, inShadowCoord ), 0.0, 0.0, 1.0 );
   outDepth  = gl_FragCoord.z;
}
