#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../LIGHTING.h"
#include "../SHADOW.h"

layout( set = 0, binding = 1 ) uniform LIGHTS { Light lights[MAX_LIGHTS]; };
layout( set = 0, binding = 2 ) uniform SHADOWMAPS { ShadowMap shadowMaps[MAX_SHADOW_MAPS]; };
layout( set = 0, binding = 3 ) uniform SHADOW_SAMPLER shadowMap;

layout( set = 1, binding = 0 ) uniform sampler2D albedo;
layout( set = 1, binding = 1 ) uniform sampler2D normals;
layout( set = 1, binding = 2 ) uniform sampler2D metalness;
layout( set = 1, binding = 3 ) uniform sampler2D roughness;
layout( set = 1, binding = 4 ) uniform sampler2D ambientOcclusion;
layout( set = 1, binding = 5 ) uniform sampler2D height;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inVSPos;
layout( location = 2 ) in vec3 inWorldPos;
layout( location = 3 ) in vec3 inTSViewDir;
layout( location = 4 ) in mat3 inTBN;

layout( location = 0 ) out vec4 outAlbedo;
layout( location = 1 ) out vec4 outNormal;
layout( location = 2 ) out vec4 outPBR;
layout( location = 3 ) out vec4 outShadow;
layout( location = 4 ) out float outDepth;

// =================================================================================================
void main()
{
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

   // Shadow Mapping
   // ============================================================================================]
   const Light sunLight = lights[0];
   const vec3 sunDir    = -sunLight.direction.xyz;

   const ShadowMap shadowMapParams = shadowMaps[0];

   float shadow = 1.0;
   if( shadowMapParams.enabled )
   {
      const vec4 shadowUV = GetShadowUV( shadowMapParams, inWorldPos, inVSPos.z, normal, sunDir );
      shadow              = ShadowFactor( shadowMap, shadowMapParams, shadowUV );
   }

   outAlbedo = vec4( albedo, 1.0 );
   outNormal = vec4( normal, 1.0 );
   outPBR    = vec4( roughness, metalness, ao, 1.0 );
   outShadow = vec4( shadow, 0.0, 0.0, 1.0 );
   outDepth  = gl_FragCoord.z;
}
