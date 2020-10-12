#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( constant_id = 0 ) const int MAX_LIGHTS = 4;

// View and environment (Alpha)
// =================================================================================================
layout( set = 0, binding = 1 ) uniform Alpha
{
   bool enabled[MAX_LIGHTS];
   vec4 lightPositions[MAX_LIGHTS];
   vec4 lightColors[MAX_LIGHTS];
   vec4 viewPos;
};

// Material properties (Gamma)
// =================================================================================================
layout( set = 1, binding = 0 ) uniform sampler2D texSampler;

layout( location = 0 ) in vec3 inTexCoords;
layout( location = 1 ) in vec3 inNormal;
layout( location = 2 ) in vec3 fragPos;

layout( location = 0 ) out vec4 outColor;

const float ambientStrength  = 0.1;
const float specularStrength = 0.5;

vec3 calcLightContribution( int i, vec3 normal )
{
   vec3 lightColor = vec3( lightColors[i] ) / 255.0;

   // Ambient
   vec3 ambient = ambientStrength * lightColor;

   // Diffuse
   vec3 lightDir = normalize( vec3( lightPositions[i] ) - fragPos );
   float diff    = max( dot( normal, lightDir ), 0.0 );
   vec3 diffuse  = diff * lightColor;

   // Specular
   vec3 specular = vec3( 0.0, 0.0, 0.0 );
   if( diff != 0.0 )  // Don't add a specular component when the diffuse dot product is 0
   {
      vec3 viewDir    = normalize( vec3( viewPos[0] ) - fragPos );
      vec3 reflectDir = reflect( -lightDir, normal );
      float spec      = pow( max( dot( normalize( viewDir ), reflectDir ), 0.0 ), 256 );
      vec3 specular   = specularStrength * spec * lightColor;
   }

   return ( ambient + diffuse + specular );
}

void main()
{
   vec3 norm         = normalize( inNormal );
   vec3 sampledColor = vec3( texture( texSampler, inTexCoords.xy ) );

   vec3 lightContributions = vec3( 0.0, 0.0, 0.0 );
   for( int i = 0; i < MAX_LIGHTS; ++i )
   {
      if( enabled[i] )
      {
         lightContributions += calcLightContribution( i, norm );
      }
   }

   outColor = vec4( sampledColor * lightContributions, 1.0 );
}
