#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "LIGHTING.h"
#include "../VIEW.h"

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };
layout( set = 1, binding = 0 ) uniform Lights { Light lights[MAX_LIGHTS]; };
layout( set = 1, binding = 1 ) uniform sampler2DShadow shadowMap;
layout( set = 1, binding = 5 ) uniform sampler2D heightMap;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inWorldPos;
layout( location = 2 ) in vec4 inShadowCoord;

layout( location = 0 ) out vec4 outColor;

// Bump mapping based on heightmap
vec3 getNormals( vec2 uv )
{
   const vec2 texelSize = 1.0 / textureSize( heightMap, 0 );
   const ivec3 off      = ivec3( -1, 0, 1 );
   const vec3 size      = vec3( 2.0 * texelSize.x, 2.0 * texelSize.y, 0.0 );

   // Uses 4 samples to create one normal, effectively "blurring" the normals
   // I don't think these are scaled properly, noon looks weird
   float hL = textureOffset( heightMap, uv, off.xy ).r;
   float hR = textureOffset( heightMap, uv, off.zy ).r;
   float hU = textureOffset( heightMap, uv, off.yx ).r;
   float hD = textureOffset( heightMap, uv, off.yz ).r;

   vec3 va = vec3( size.x, hR - hL, size.z );
   vec3 vb = vec3( size.z, hU - hD, -size.y );

   /*
   // 3 samples variant
   float hO = texture(heightMap, uv).r;
   float hR = textureOffset( heightMap, uv, off.zy ).r;
   float hU = textureOffset( heightMap, uv, off.yx ).r;

   vec3 va = (vec3(size.x, hR - hO, size.y));
   vec3 vb = (vec3(size.y, hU - hO, -size.x));
   */

   return normalize( cross( va, vb ) );
}

// =================================================================================================
void main()
{
   // Determine terrain diffuse color
   vec3 bottomColor = vec3( 0.0, 1.0, 0.0 );
   vec3 topColor    = vec3( 1.0, 1.0, 1.0 );

   vec3 unlitColor = mix( bottomColor, topColor, inWorldPos.y / 10.0 );
   if( inWorldPos.y < 0.1 )
   {
      unlitColor = vec3( 0.05, 1.0, 1.0 );
   }

   const View mainView = views[0];

   const vec3 normal = getNormals( inUV );

   const Light curLight = lights[0];
   const vec3 lightDir  = normalize( curLight.position.xyz );
   const vec3 viewDir   = normalize( mainView.pos.xyz - inWorldPos );

   const vec3 ambientTerm = curLight.color.rgb * ConstantAmbient();
   const float sunFactor =
       clamp( smoothstep( 0.0, 1.0, dot( lightDir, vec3( 0.0, 1.0, 0.0 ) ) ), 0.0, 1.0 );

   const vec3 nightColor = unlitColor * ambientTerm;

   vec3 finalColor = nightColor;
   if( sunFactor > 0.0 && curLight.params.x > 0.0 )
   {
      const float shadow     = ShadowPCF( shadowMap, inShadowCoord );
      const vec3 diffuseTerm = curLight.color.rgb * LambertianDiffuse( lightDir, normal ) * shadow;
      const vec3 specularTerm =
          curLight.color.rgb * BlinnPhongSpecular( lightDir, viewDir, normal ) * shadow;

      // Adding diffuse and specular terms to the day color
      const vec3 dayColor = nightColor + unlitColor * ( diffuseTerm + specularTerm );

      // Mix between fully lit day color and ambient-only night color based on sun presence
      finalColor = mix( nightColor, dayColor, sunFactor );
   }

   outColor = vec4( finalColor, 1.0 );
}
