#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "LIGHTING.h"
#include "../VIEW.h"

layout( set = 0, binding = 0 ) uniform Views { View views[MAX_VIEWS]; };
layout( set = 1, binding = 0 ) uniform Lights { Light lights[MAX_LIGHTS]; };
//layout( set = 1, binding = 2 ) uniform sampler2DShadow shadowMap;

layout( location = 0 ) in vec2 inUV;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 inWorldPos;
layout( location = 3 ) in vec4 inShadowCoord;

layout( location = 0 ) out vec4 outColor;

// =================================================================================================
void main()
{
   vec3 unlitColor = vec3( 1.0, 0.0, 0.0 );

   const View mainView = views[0];

   const vec3 normal = normalize( inNormal );

   const Light curLight = lights[0];
   const vec3 lightDir  = normalize( curLight.position.xyz - inWorldPos );
   const vec3 viewDir   = normalize( mainView.pos.xyz - inWorldPos );

   const vec3 ambientTerm = curLight.color.rgb * ConstantAmbient();
   const float sunFactor =
       clamp( smoothstep( 0.0, 1.0, dot( lightDir, vec3( 0.0, 1.0, 0.0 ) ) ), 0.0, 1.0 );

   const vec3 nightColor = unlitColor * ambientTerm;

   vec3 finalColor = nightColor;
   if( sunFactor > 0.0 && curLight.params.x > 0.0 )
   {
      const float shadow     = 1.0;//ShadowPCF( shadowMap, inShadowCoord, normal, inWorldPos );
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
