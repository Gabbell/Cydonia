#version 450
#extension GL_ARB_separate_shader_objects : enable

#include "../LIGHTING.h"
#include "../VIEW.h"

layout( set = 0, binding = 1 ) uniform Views { View views[MAX_VIEWS]; };
layout( set = 0, binding = 2 ) uniform InverseViews { InverseView inverseViews[MAX_VIEWS]; };
layout( set = 0, binding = 3 ) uniform Lights { Light lights[MAX_LIGHTS]; };

layout( set = 1, binding = 0 ) uniform sampler2D albedo;
layout( set = 1, binding = 1 ) uniform sampler2D normals;
layout( set = 1, binding = 2 ) uniform sampler2D shadowMask;
layout( set = 1, binding = 3 ) uniform sampler2D depth;

layout( location = 0 ) in vec2 inUV;

layout( location = 0 ) out vec4 outColor;

void main()
{
   const vec3 unlitColor = texture( albedo, inUV ).rgb;
   const vec3 normal     = texture( normals, inUV ).xyz;
   const vec4 shadow     = texture( shadowMask, inUV );
   const float depth     = texture( depth, inUV ).r;

   // Views
   const View mainView               = views[0];
   const InverseView inverseMainView = inverseViews[0];

   // Reconstruct world-space coordinates
   // Using view-space coordinates is not enough because it doesn't consider
   // the current view's rotation
   const vec4 ndc = vec4( inUV * 2.0 - 1.0, depth, 1.0 );
   vec4 worldPos  = inverseMainView.invView * inverseMainView.invProj * ndc;
   worldPos       = worldPos / worldPos.w;

   const Light curLight = lights[0];
   const vec3 lightDir  = normalize( curLight.position.xyz );
   const vec3 fragToView   = normalize( mainView.pos.xyz - worldPos.xyz );

   const vec3 ambientTerm = curLight.color.rgb * ConstantAmbient();
   const float sunFactor =
       clamp( smoothstep( 0.0, 0.08, dot( lightDir, vec3( 0.0, 1.0, 0.0 ) ) ), 0.0, 1.0 );

   const vec3 nightColor = unlitColor * ambientTerm;

   vec3 finalColor = nightColor;
   if( sunFactor > 0.0 && curLight.params.x > 0.0 )
   {
      const vec3 diffuseTerm = curLight.color.rgb * LambertianDiffuse( lightDir, normal ) * shadow.r;
      const vec3 specularTerm =
          curLight.color.rgb * BlinnPhongSpecular( lightDir, fragToView, normal ) * shadow.r;

      // Adding diffuse and specular terms to the day color
      const vec3 dayColor = nightColor + unlitColor * ( diffuseTerm + specularTerm );

      // Mix between fully lit day color and ambient-only night color based on sun presence
      finalColor = mix( nightColor, dayColor, sunFactor );
   }

   outColor = vec4( finalColor, 1.0 );
}
