#version 450
#extension GL_ARB_separate_shader_objects : enable

// Cubemap Texture
layout( set = 1, binding = 0 ) uniform samplerCube cubemapTexture;

layout( location = 0 ) in vec4 inColor;
layout( location = 1 ) in vec3 inTexCoords;

layout( location = 0 ) out vec4 outColor;

void main() { outColor = inColor * texture( cubemapTexture, inTexCoords ); }
