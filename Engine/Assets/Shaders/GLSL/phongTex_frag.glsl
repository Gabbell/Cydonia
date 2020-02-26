#version 450
#extension GL_ARB_separate_shader_objects : enable

// View and environment
// =================================================================================================
layout( set = 0, binding = 1 ) uniform Alpha
{
    vec4 viewPos;
    vec4 lightPos;
    vec4 lightCol;
};

// Shader control values
// =================================================================================================
// layout( set = 1, binding = 0 ) uniform Beta
// {
//     vec4 control;
// };

// Material properties
// =================================================================================================
layout( set = 1, binding = 0 ) uniform Gamma
{
    vec4 mat;
};
layout( set = 1, binding = 1 ) uniform sampler2D texSampler;

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec3 inTexCoord;
layout( location = 2 ) in vec3 inNormal;
layout( location = 3 ) in vec3 fragPos;

layout( location = 0 ) out vec4 outColor;

float ambientStrength = 0.1;
float specularStrength = 0.5;

void main() {
   vec3 norm = normalize( inNormal );

   // Ambient
   vec3 ambient = ambientStrength * vec3( lightCol );

   // Diffuse
   vec3 lightDir = normalize( vec3( lightPos ) - fragPos );
   float diff = max( dot( norm, lightDir ), 0.0 );
   vec3 diffuse = diff * vec3( lightCol );

   // Specular
   vec3 viewDir = normalize( vec3( viewPos ) - fragPos );
   vec3 reflectDir = reflect( -lightDir, norm );  
   float spec = pow( max( dot( normalize( viewDir ), reflectDir ), 0.0), 256 );
   vec3 specular = specularStrength * spec * vec3( lightCol );  
   
   vec3 sampledColor = vec3( texture( texSampler, inTexCoord.xy ) );
   vec3 result = ( ambient + diffuse + specular ) * sampledColor;

   outColor = vec4( result, 1.0 );
}

