#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../VIEW.h"
#include "../LIGHTING.h"

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };
layout( set = 0, binding = 1 ) uniform sampler2DShadow shadowMap;

layout( set = 1, binding = 0 ) uniform sampler2D albedo;
layout( set = 1, binding = 1 ) uniform sampler2D normals;
layout( set = 1, binding = 2 ) uniform sampler2D metalness;
layout( set = 1, binding = 3 ) uniform sampler2D roughness;
layout( set = 1, binding = 4 ) uniform sampler2D ambientOcclusion;
layout( set = 1, binding = 5 ) uniform sampler2D height;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inShadowCoord;
layout( location = 2 ) in vec3 inTSViewDir;
layout( location = 3 ) in mat3 inTBN;

layout( location = 0 ) out vec4 outAlbedo;
layout( location = 1 ) out vec4 outNormal;
layout( location = 2 ) out vec4 outPBR;
layout( location = 3 ) out vec4 outShadow;
layout( location = 4 ) out float outDepth;

// =================================================================================================
void main()
{
   const View mainView = views[0];

   vec2 uv = inUV;

   // Parallax Occlusion Mapping
   // TODO better POM detection, probably just a constant buffer
   if( textureSize( height, 0 ).x > 1 )
   {
      const vec3 TSviewDir = normalize( inTSViewDir );
      const vec3 TSnormal =
          normalize( textureLod( normals, inUV, 0 ).xyz * 2.0 - 1.0 );  // Tangent space
      uv = POMDown( height, inUV, TSviewDir, TSnormal );
      if( uv.x > 1.0 || uv.x < 0.0 || uv.y > 1.0 || uv.y < 0.0 ) discard;
   }

   const vec3 TSnormal   = normalize( texture( normals, uv ).xyz * 2.0 - 1.0 );
   const vec3 albedo     = texture( albedo, uv ).rgb;
   const float metalness = texture( metalness, uv ).r;
   const float roughness = texture( roughness, uv ).r;
   const float ao        = texture( ambientOcclusion, uv ).r;

   const vec3 normal = normalize( inTBN * TSnormal );

   outAlbedo = vec4( albedo, 1.0 );
   outNormal = vec4( normal, 1.0 );
   outPBR    = vec4( roughness, metalness, ao, 0.0 );
   outShadow = vec4( ShadowPCF( shadowMap, inShadowCoord ), 0.0, 0.0, 1.0 );
   outDepth  = gl_FragCoord.z;
}
